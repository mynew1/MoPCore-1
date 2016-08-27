

#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "CreatureTextMgr.h"
#include "SpellScript.h"
#include "SpellAuras.h"
#include "SpellAuraEffects.h"
#include "Player.h"

enum Texts
{
    // Dohaman the Beast Lord.
    SAY_INTRO_1            = 0,      // How dare you interupt our preparations! The Zandalari will not be stopped! Not this time!
    SAY_INTRO_2            = 1,      // Destroy them! I command you! No, stop, STOP!
    SAY_INTRO_DONE         = 2       // Oondasta eats Dohaman.

    // No texts for Oondasta.
};

enum Spells
{
    // Oondasta.
    SPELL_CRUSH            = 137504, // Damage + Armor reduce.
    SPELL_ALPHA_MALE       = 138391, // Immune to Taunts, Threat multiplier trigger.
    SPELL_ALPHA_MALE_TM    = 138390, // Threat multiplier.
    SPELL_FRILL_BLAST      = 137505, // Damage, frontal cone.
    SPELL_GROWING_FURY     = 137502, // Damage done increase.
    SPELL_PIERCING_ROAR    = 137457, // Interrupt + damage.
    SPELL_SPIRITFIRE_BEAM  = 137508, // Also 137511, damage + jump. Second spell id does exactly the same thing.

    SPELL_KILL_DOHAMAN     = 138859, // "Eat" instakill.
};

enum Events
{
    // Oondasta.
    EVENT_CRUSH            = 1, // 60s from aggro, 25-30s after.
    EVENT_PIERCING_ROAR    = 2, // 20s from aggro, 30-50s after.
    EVENT_FRILL_BLAST      = 3, // 40s from aggro, 25-30s after.
    EVENT_SPIRITFIRE_BEAM  = 4, // 50s from aggro, 25-30s after.
    EVENT_GROWING_FURY     = 5, // Stacks roughly every 30s.

    EVENT_INTRO_1,
    EVENT_INTRO_2,
    EVENT_INTRO_DONE
};

enum Npcs
{
    NPC_DOHAMAN_BEAST_LORD = 69926
};

class boss_oondasta : public CreatureScript
{
    public:
        boss_oondasta() : CreatureScript("boss_oondasta") { }

        struct boss_oondastaAI : public ScriptedAI
        {
            boss_oondastaAI(Creature* creature) : ScriptedAI(creature), summons(me)
            {
                introDone = false;
			}

            EventMap events;
            SummonList summons;
            bool introDone;

            void InitializeAI()
            {
                if (!me->isDead())
                    Reset();
            }

            void Reset()
            {
                events.Reset();
                summons.DespawnAll();

                // If the intro isn't done, summon Dohaman and set his Orientation.
                if (!introDone)
                {
                    float x, y, z;
                    me->GetClosePoint(x, y, z, me->GetObjectSize() / 3, 10.0f);
                    if (Creature* Dohaman = me->SummonCreature(NPC_DOHAMAN_BEAST_LORD, x, y, z, 0.0f, TEMPSUMMON_MANUAL_DESPAWN))
                        Dohaman->SetFacingTo(me->GetOrientation());
                }
            }

            void EnterCombat(Unit* /*who*/)
            {
                if (!introDone)
                {
                    if (Creature* Dohaman = me->FindNearestCreature(NPC_DOHAMAN_BEAST_LORD, 50.0f, true))
                    {
                        Dohaman->SetInCombatWithZone();
                        me->SetFacingToObject(Dohaman);
                        me->AddUnitState(UNIT_STATE_ROOT | UNIT_STATE_CANNOT_TURN);
                        me->SetReactState(REACT_PASSIVE);
				    }

                    events.ScheduleEvent(EVENT_INTRO_1, 1000);
                }
                else
                {
                    // Intro done, add the aura and schedule the events.
                    me->AddAura(SPELL_ALPHA_MALE, me);

                    events.ScheduleEvent(EVENT_CRUSH, 60000);
                    events.ScheduleEvent(EVENT_PIERCING_ROAR, 20000);
                    events.ScheduleEvent(EVENT_FRILL_BLAST, 40000);
                    events.ScheduleEvent(EVENT_SPIRITFIRE_BEAM, 50000);
                    events.ScheduleEvent(EVENT_GROWING_FURY, 30000);
                }
            }

            void KilledUnit(Unit* victim) { }

            void EnterEvadeMode()
            {
                me->RemoveAllAuras();
                Reset();
                me->DeleteThreatList();
                me->CombatStop(true);
                me->GetMotionMaster()->MoveTargetedHome();
            }

            void JustDied(Unit* /*killer*/)
            {
                summons.DespawnAll();
            }

            void JustSummoned(Creature* summon)
            {
                summons.Summon(summon);
                summon->setActive(true);
            }

            void UpdateAI(const uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_INTRO_1:
                            if (Creature* Dohaman = me->FindNearestCreature(NPC_DOHAMAN_BEAST_LORD, 50.0f, true))
                                Dohaman->AI()->Talk(SAY_INTRO_1);
                            events.ScheduleEvent(EVENT_INTRO_2, 8500);
                            break;

                        case EVENT_INTRO_2:
                            if (Creature* Dohaman = me->FindNearestCreature(NPC_DOHAMAN_BEAST_LORD, 50.0f, true))
					        {
                                Dohaman->SetFacingToObject(me);
                                Dohaman->AI()->Talk(SAY_INTRO_2);
					        }
                            events.ScheduleEvent(EVENT_INTRO_DONE, 8000);
                            break;

                        case EVENT_INTRO_DONE:
                            if (Creature* Dohaman = me->FindNearestCreature(NPC_DOHAMAN_BEAST_LORD, 50.0f, true))
                            {
                                Dohaman->AI()->Talk(SAY_INTRO_DONE);
                                me->SetFacingToObject(Dohaman);
                                DoCast(Dohaman, SPELL_KILL_DOHAMAN, true);
                            }

                            // Intro done, add the aura and schedule the events.
                            introDone = true;
                            me->AddAura(SPELL_ALPHA_MALE, me);
                            me->ClearUnitState(UNIT_STATE_ROOT | UNIT_STATE_CANNOT_TURN);
                            me->SetReactState(REACT_AGGRESSIVE);

                            events.ScheduleEvent(EVENT_CRUSH, 60000);
                            events.ScheduleEvent(EVENT_PIERCING_ROAR, 20000);
                            events.ScheduleEvent(EVENT_FRILL_BLAST, 40000);
                            events.ScheduleEvent(EVENT_SPIRITFIRE_BEAM, 50000);
                            events.ScheduleEvent(EVENT_GROWING_FURY, 30000);
                            break;

                        case EVENT_CRUSH:
                            DoCast(me->getVictim(), SPELL_CRUSH);
                            events.ScheduleEvent(EVENT_CRUSH, urand(25000, 30000));
                            break;

                        case EVENT_PIERCING_ROAR:
                            DoCast(me, SPELL_PIERCING_ROAR);
                            events.ScheduleEvent(EVENT_PIERCING_ROAR, urand(30000, 50000));
                            break;

                        case EVENT_FRILL_BLAST:
                            DoCast(me, SPELL_FRILL_BLAST);
                            events.ScheduleEvent(EVENT_FRILL_BLAST, urand(25000, 30000));
                            break;

                        case EVENT_SPIRITFIRE_BEAM:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, NonTankTargetSelector(me)))
                                DoCast(target, SPELL_SPIRITFIRE_BEAM);
                            events.ScheduleEvent(EVENT_SPIRITFIRE_BEAM, urand(25000, 30000));
                            break;

                        case EVENT_GROWING_FURY:
                            DoCast(me, SPELL_GROWING_FURY);
                            events.ScheduleEvent(EVENT_GROWING_FURY, 30000);
                            break;

                        default: break;
                    }
                }

                if (introDone)
                    DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_oondastaAI(creature);
        }
};

void AddSC_boss_oondasta()
{
    new boss_oondasta();
}
