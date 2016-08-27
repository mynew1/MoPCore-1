
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
    // Niuzao
    SAY_INTRO               = 0, // Can you stand on the tallest peak? Winds and sleet buffeting your skin, until the trees wither and the mountains fall into the sea?
    SAY_AGGRO               = 1, // We shall see.
    SAY_DEATH               = 2, // Though you will be surrounded by foes greater than you can imagine, your fortitude shall allow you to endure. Remember this in the times ahead.
    SAY_SLAY                = 3, // You must persevere!
    SAY_MASSIVE_QUAKE       = 4, // Be vigilant in your stand or you will never achieve your goals!
    SAY_CHARGE              = 5  // 0 - The winds may be strong, and the sleets may sting. ; 1 - You are the mountain unmovable by all but time!
};

enum Spells
{
    // Niuzao
    SPELL_HEADBUTT          = 144610, // Damage, knockback and threat removal.
    SPELL_OXEN_FORTITUDE    = 144606, // Player health + boss damage increase.
    SPELL_OXEN_FORTITUDE_T  = 144607, // Triggered on players by above.
    SPELL_MASSIVE_QUAKE     = 144611, // Damage each sec.
    SPELL_MASSIVE_QUAKE_D   = 144612, // Triggered by above.
    SPELL_CHARGE            = 144608, // Charge cast time and aura.
    SPELL_CHARGE_D          = 144609, // Per. dmg., triggered by above each sec.
};

enum Events
{
    // Niuzao
    EVENT_HEADBUTT          = 1, // 20s from aggro, 30s after.
    EVENT_OXEN_FORTITUDE    = 2, // 12s from aggro, 45s after.
    EVENT_MASSIVE_QUAKE     = 3, // 45s from aggro, 65s after.
    EVENT_NIUZAO_CHARGE     = 4  // 66 and 33%.
};

enum ChargeStates
{
    DONE_NONE               = 0, // No casts done.
    DONE_66                 = 1, // First cast done.
    DONE_33                 = 2  // Both casts done.
};

// ToDo: Script Charge, fix timers.
class boss_niuzao : public CreatureScript
{
    public:
        boss_niuzao() : CreatureScript("boss_niuzao") { }

        struct boss_niuzaoAI : public ScriptedAI
        {
            boss_niuzaoAI(Creature* creature) : ScriptedAI(creature) { }

            EventMap events;
            uint8 chargeDone;

            void InitializeAI()
            {
                if (!me->isDead())
                    Reset();
            }

            void Reset()
            {
                events.Reset();

                chargeDone = DONE_NONE;
            }
    
            void EnterCombat(Unit* /*who*/)
            {
                Talk(SAY_AGGRO);

                events.ScheduleEvent(EVENT_HEADBUTT, urand(18000, 23000)); // 18-23
                events.ScheduleEvent(EVENT_OXEN_FORTITUDE, urand(12000, 14000)); // 12-14
                events.ScheduleEvent(EVENT_MASSIVE_QUAKE, urand(44000, 48000)); // 44-48
            }

            void KilledUnit(Unit* victim)
            {
                if (victim->GetTypeId() == TYPEID_PLAYER)
                    Talk(SAY_SLAY);
            }

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
                Talk(SAY_DEATH);
            }

            void UpdateAI(const uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                // Set Crane Rush phases execution.
                if (me->HealthBelowPct(67) && chargeDone == DONE_NONE || me->HealthBelowPct(34) && chargeDone == DONE_66)
                {
                    events.ScheduleEvent(EVENT_NIUZAO_CHARGE, 2000);
                    chargeDone++;
                }

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_HEADBUTT:
                            DoCastVictim(SPELL_HEADBUTT);
                            events.ScheduleEvent(EVENT_HEADBUTT, urand(35000, 40000));
                            break;

                        case EVENT_OXEN_FORTITUDE:
                            DoCast(me, SPELL_OXEN_FORTITUDE);
                            events.ScheduleEvent(EVENT_OXEN_FORTITUDE, urand(43000, 47000));
                            break;

                        case EVENT_MASSIVE_QUAKE:
                            Talk(SAY_MASSIVE_QUAKE);
                            DoCast(me, SPELL_MASSIVE_QUAKE);
                            events.ScheduleEvent(EVENT_MASSIVE_QUAKE, urand(70000, 75000));
                            break;

                        case EVENT_NIUZAO_CHARGE:
                            Talk(SAY_CHARGE);
                            DoCast(me, SPELL_CHARGE);
                            break;

                        default: break;
                    }
                }

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_niuzaoAI(creature);
        }
};

void AddSC_boss_niuzao()
{
    new boss_niuzao();
}
