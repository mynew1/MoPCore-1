

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
    // Ordos.
    SAY_AGGRO               = 0,      // You will take my place on the eternal brazier.
    SAY_DEATH               = 1,      // The eternal fire will never be extinguished.
    SAY_SLAY                = 2,      // Up in smoke.

    SAY_ANCIENT_FLAME       = 3,      // Your flesh will melt.
    SAY_ETERNAL_AGONY       = 4,      // Your pain will be endless.
    SAY_POOL_OF_FIRE        = 5,      // You will feel but a fraction of my agony.
    SAY_BURNING_SOUL        = 6       // Burn!
};

enum Spells
{
    // Ordos.
    SPELL_MAGMA_CRUSH       = 144688, // 10s, every 15 s.
    SPELL_ANCIENT_FLAME_S   = 144695, // 40s
    SPELL_ANCIENT_FLAME_A   = 144691, // Periodic dmg. aura.
    SPELL_ANCIENT_FLAME_D   = 144699, // Dmg spell.
    SPELL_BURNING_SOUL      = 144689, // 20s, every 30 s.
    SPELL_BURNING_SOUL_EX   = 144690, // Removal Explode spell.
    SPELL_POOL_OF_FIRE      = 144692, // 30s - Summon spell.
    SPELL_POOL_OF_FIRE_A    = 144693, // Periodic dmg. aura.

    SPELL_ETERNAL_AGONY     = 144696  // Enrage spell, 5 min.
};

enum Events
{
    // Ordos.
    EVENT_ANCIENT_FLAME    = 1,
    EVENT_BURNING_SOUL,
    EVENT_POOL_OF_FIRE,
    EVENT_MAGMA_CRUSH,

    EVENT_ETERNAL_AGONY               // Berserk.
};

enum Npcs
{
    NPC_ANCIENT_FLAME      = 72059
};

class boss_ordos : public CreatureScript
{
    public:
        boss_ordos() : CreatureScript("boss_ordos") { }

        struct boss_ordosAI : public ScriptedAI
        {
            boss_ordosAI(Creature* creature) : ScriptedAI(creature), summons(me) { }

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

                events.ScheduleEvent(EVENT_MAGMA_CRUSH, 10000);                 // 10-15s
                events.ScheduleEvent(EVENT_ANCIENT_FLAME, 40000);               // 40-45s
                events.ScheduleEvent(EVENT_BURNING_SOUL, 20000);                // 20-30s
                events.ScheduleEvent(EVENT_POOL_OF_FIRE, 30000);                // 30-40s

                events.ScheduleEvent(EVENT_ETERNAL_AGONY, 300000);              // Berserk. After 5 minutes.
            }

            void KilledUnit(Unit* victim)
            {
                if (victim->GetTypeId() == TYPEID_PLAYER)
                    Talk(SAY_SLAY);
            }

            void EnterEvadeMode()
            {
                me->RemoveAllAuras();
                me->RemoveAllAreasTrigger();
                Reset();
                me->DeleteThreatList();
                me->CombatStop(true);
                me->GetMotionMaster()->MoveTargetedHome();
            }

            void JustDied(Unit* /*killer*/)
            {
                Talk(SAY_DEATH);
                summons.DespawnAll();
				me->RemoveAllAreasTrigger();
            }

            void JustSummoned(Creature* summon)
            {
                summons.Summon(summon);
                summon->setActive(true);

		        if (me->isInCombat())
                    summon->SetInCombatWithZone();

                if (summon->GetEntry() == NPC_ANCIENT_FLAME)
                {
                    summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_DISABLE_MOVE);
                    summon->AddUnitState(UNIT_STATE_CANNOT_TURN);
                    summon->SetReactState(REACT_PASSIVE);
                    summon->AddAura(SPELL_ANCIENT_FLAME_A, summon);
                }
            }

            void UpdateAI(const uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                // While they are alive, the Ancient Flames must always have the Periodic aura.
                std::list<Creature*> searcher;
                GetCreatureListWithEntryInGrid(searcher, me, NPC_ANCIENT_FLAME, 100.0f);
                for (auto aFlame : searcher)
                {
                    if (!aFlame)
                        continue;

                    if (!aFlame->HasAura(SPELL_ANCIENT_FLAME_A))
                        aFlame->AddAura(SPELL_ANCIENT_FLAME_A, aFlame);
                }

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_MAGMA_CRUSH:
                            DoCastVictim(SPELL_MAGMA_CRUSH);
                            events.ScheduleEvent(EVENT_MAGMA_CRUSH, urand(15000, 20000));
                            break;

                        case EVENT_ANCIENT_FLAME:
                            Talk(SAY_ANCIENT_FLAME);
                            DoCast(me, SPELL_ANCIENT_FLAME_S);
                            events.ScheduleEvent(EVENT_ANCIENT_FLAME, urand(40000, 45000));
                            break;

                        case EVENT_POOL_OF_FIRE:
                            Talk(SAY_POOL_OF_FIRE);
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, NonTankTargetSelector(me)))
                                DoCast(target, SPELL_POOL_OF_FIRE);
                            events.ScheduleEvent(EVENT_POOL_OF_FIRE, urand(30000, 40000));
                            break;

                        case EVENT_BURNING_SOUL:
                            Talk(SAY_BURNING_SOUL);
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, NonTankTargetSelector(me)))
                                DoCast(target, SPELL_BURNING_SOUL);
                            events.ScheduleEvent(EVENT_BURNING_SOUL, urand(20000, 30000));
                            break;

                        case EVENT_ETERNAL_AGONY: // Berserk.
                            Talk(SAY_ETERNAL_AGONY);
                            DoCast(me, SPELL_ETERNAL_AGONY);
                            events.ScheduleEvent(EVENT_ETERNAL_AGONY, 20000); // If anyone is left alive (bubble etc.) :).
                            break;

                        default: break;
                    }
                }

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_ordosAI(creature);
        }
};

// Ordos Burning Soul 144689.
class spell_ordos_burning_soul_aura : public SpellScriptLoader
{
    public:
        spell_ordos_burning_soul_aura() : SpellScriptLoader("spell_ordos_burning_soul_aura") { }

        class spell_ordos_burning_soul_aura_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_ordos_burning_soul_aura_AuraScript);

            void OnRemove(constAuraEffectPtr /*aurEff*/, AuraEffectHandleModes mode)
            {
                if (!(mode & AURA_EFFECT_HANDLE_REAL))
                    return;

                Unit* target = GetTarget();

                if (!target || !GetTargetApplication() || !GetAura())
                    return;

                if (target->GetTypeId() != TYPEID_PLAYER)
                    return;

                // Only on removal by expire.
                if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_EXPIRE)
                    return;

                target->CastSpell(target, SPELL_BURNING_SOUL_EX, true);
            }

            void Register()
            {
                OnEffectRemove += AuraEffectRemoveFn(spell_ordos_burning_soul_aura_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_ordos_burning_soul_aura_AuraScript();
        }
};

void AddSC_boss_ordos()
{
    new boss_ordos();
    new spell_ordos_burning_soul_aura();
}
