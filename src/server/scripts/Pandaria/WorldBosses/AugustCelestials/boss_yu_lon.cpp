
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
    SAY_INTRO               = 0, // The lesson of true wisdom lies within you, dear heroes, and in the choices you make. When faced with overwhelming darkness, will you make the right decisions?
    SAY_AGGRO               = 1, // The trial begins!
    SAY_DEATH               = 2, // Your wisdom has seen you through this trial. May it ever light your way out of dark places.
    SAY_SLAY                = 3, // Learn from your mistakes.
    SAY_JADEFIRE_WALL       = 4  // 0 - Listen to your inner voice, and seek out the truth. ; 1 - Do not let your judgement be clouded in trying times. ; 2 - Always consider the consequences of your actions.
};

enum Spells
{
    // Boss
    SPELL_JADEFLAME_BUFFET  = 144630, // Damage spell + increase % dmg taken.
    SPELL_JADEFIRE_BREATH   = 144530, // Damage spell.
    SPELL_JADEFIRE_BOLT     = 144545, // Cast time + spell. Script Effect to cast in 50y.
    SPELL_JADEFIRE_BOLT_S   = 144532, // On above SE. Triggers missiles in 50y.
    SPELL_JADEFIRE_BOLT_M   = 144541, // Triggered by above. Spawns NPC_JADEFIRE_BOLT on impact point.
    SPELL_JADEFIRE_WALL     = 144533, // Creates Areatrigger 1087.
    SPELL_JADEFIRE_WALL_DMG = 144539, // Damage from above Areatrigger.

    // NPCs
    SPELL_JADEFIRE_BOLT_D   = 147454  // Damage from standing within 11y of NPC_JADEFIRE_BOLT spawns.
};

enum Events
{
    EVENT_JADEFLAME_BUFFET  = 1, // 20s from aggro, 30s after.
    EVENT_JADEFIRE_BREATH   = 2, // 12s from aggro, 45s after.
    EVENT_JADEFIRE_BOLT     = 3, // 45s from aggro, 65s after.
    EVENT_JADEFIRE_WALL     = 4
};

enum Npcs
{
    NPC_JADEFIRE_BOLT       = 72016
};

// ToDo: Script Jadefire Bolt + NPC_JADEFIRE_BOLT, Jadefire Wall Areatrigger, fix timers.
class boss_yu_lon : public CreatureScript
{
    public:
        boss_yu_lon() : CreatureScript("boss_yu_lon") { }

        struct boss_yu_lonAI : public ScriptedAI
        {
            boss_yu_lonAI(Creature* creature) : ScriptedAI(creature), summons(me) { }

            EventMap events;
            SummonList summons;

            void InitializeAI()
            {
                if (!me->isDead())
                    Reset();
            }

            void Reset()
            {
                events.Reset();
                summons.DespawnAll();
            }
    
            void EnterCombat(Unit* /*who*/)
            {
                Talk(SAY_AGGRO);

                events.ScheduleEvent(EVENT_JADEFLAME_BUFFET, urand(18000, 23000)); // 18-23
                events.ScheduleEvent(EVENT_JADEFIRE_BREATH, urand(12000, 14000)); // 12-14
                events.ScheduleEvent(EVENT_JADEFIRE_BOLT, urand(24000, 30000)); // 44-48
                events.ScheduleEvent(EVENT_JADEFIRE_WALL, urand(44000, 48000)); // 44-48
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
                summons.DespawnAll();
            }

            void JustSummoned(Creature* summon)
            {
                summons.Summon(summon);
                summon->setActive(true);

		        if (me->isInCombat())
                    summon->SetInCombatWithZone();

                switch (summon->GetEntry())
                {
                    case NPC_JADEFIRE_BOLT:
                        summon->SetReactState(REACT_PASSIVE);
						summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                        break;

                    default: break;
                }
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
                        case EVENT_JADEFLAME_BUFFET:
                            DoCast(me, SPELL_JADEFLAME_BUFFET);
                            events.ScheduleEvent(EVENT_JADEFLAME_BUFFET, urand(30000, 40000));
                            break;

                        case EVENT_JADEFIRE_BREATH:
                            DoCast(me, SPELL_JADEFIRE_BREATH);
                            events.ScheduleEvent(EVENT_JADEFIRE_BREATH, urand(20000, 25000));
                            break;

                        case EVENT_JADEFIRE_BOLT:
                            DoCast(me, SPELL_JADEFIRE_BOLT);
                            events.ScheduleEvent(EVENT_JADEFIRE_BOLT, urand(50000, 60000));
                            break;

                        case EVENT_JADEFIRE_WALL:
                            Talk(SAY_JADEFIRE_WALL);
                            DoCast(me, SPELL_JADEFIRE_WALL);
                            events.ScheduleEvent(EVENT_JADEFIRE_WALL, urand(80000, 90000));
                            break;

                        default: break;
                    }
                }

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_yu_lonAI(creature);
        }
};

void AddSC_boss_yu_lon()
{
    new boss_yu_lon();
}
