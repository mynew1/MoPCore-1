

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
    // Xuen
    SAY_INTRO               = 0, // Strength is far more than simple physical prowess. When you are truly tested, will you be able to tell the difference between strength and power?
    SAY_AGGRO               = 1, // Haha! The trial commences.
    SAY_DEATH               = 2, // You are strong, stronger even than you realize. Carry this thought with you into the darkness ahead. Let it shield you.
    SAY_SLAY                = 3, // Return twice as powerful.
    SAY_AGILITY             = 4  // 0 - Believe in your strength. ; 1 - You have the power to change your destiny. ; 2 - Do not mistake the power that darkness offers for true strength.
};

enum Spells
{
    // Xuen
    SPELL_LEAP              = 144640, // Jump spell.
    SPELL_SPECTRAL_SWIPE    = 144652, // Damage and armor reduction. Triggered by above.
    SPELL_CRACK_LIGHTNING   = 144635, // Cast time and aura.
    SPELL_CRACK_LIGHTNING_S = 144634, // Script effect to cast each sec in 60y. Triggered by above.
    SPELL_CRACK_LIGHTNING_D = 144633, // Damage, on above SE. Jumps to 5 targets.
    SPELL_CHI_BARRAGE       = 144642, // Cast time and aura. Script effect to cast in 100y.
    SPELL_CHI_BARRAGE_M     = 144643, // Triggers missiles.
    SPELL_CHI_BARRAGE_D     = 144644, // Damage spell for impact zone, 3y. Triggered by above.
    SPELL_AGILITY           = 144631  // Boss buff spell.
};

enum Events
{
    // Xuen
    EVENT_LEAP              = 1, // 20s from aggro, 30s after.
    EVENT_CRACK_LIGHTNING   = 2, // 12s from aggro, 45s after.
    EVENT_CHI_BARRAGE       = 3, // 45s from aggro, 65s after.
    EVENT_AGILITY           = 4
};

// ToDo : Script Crackling Lightning + Chi Barrage, fix timers.
class boss_xuen : public CreatureScript
{
    public:
        boss_xuen() : CreatureScript("boss_xuen") { }

        struct boss_xuenAI : public ScriptedAI
        {
            boss_xuenAI(Creature* creature) : ScriptedAI(creature) { }

            EventMap events;

            void InitializeAI()
            {
                if (!me->isDead())
                    Reset();
            }

            void Reset()
            {
                events.Reset();
            }

            void EnterCombat(Unit* /*who*/)
            {
                Talk(SAY_AGGRO);

                events.ScheduleEvent(EVENT_CRACK_LIGHTNING, urand(18000, 23000));
                events.ScheduleEvent(EVENT_LEAP, urand(12000, 14000));
                events.ScheduleEvent(EVENT_CHI_BARRAGE, urand(44000, 48000));
                events.ScheduleEvent(EVENT_AGILITY, urand(60000, 70000));
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

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_LEAP:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, NonTankTargetSelector(me)))
                                DoCast(target, SPELL_LEAP);
                            events.ScheduleEvent(EVENT_LEAP, urand(35000, 40000));
                            break;

                        case EVENT_CRACK_LIGHTNING:
                            DoCast(me, SPELL_CRACK_LIGHTNING);
                            events.ScheduleEvent(EVENT_CRACK_LIGHTNING, urand(35000, 40000));
                            break;

                        case EVENT_CHI_BARRAGE:
                            DoCast(me, SPELL_CHI_BARRAGE);
                            events.ScheduleEvent(EVENT_CHI_BARRAGE, urand(43000, 47000));
                            break;

                        case EVENT_AGILITY:
                            Talk(SAY_AGILITY);
                            DoCast(me, SPELL_AGILITY);
                            events.ScheduleEvent(EVENT_AGILITY, urand(113000, 127000));
                            break;

                        default: break;
                    }
                }

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_xuenAI(creature);
        }
};

void AddSC_boss_xuen()
{
    new boss_xuen();
}
