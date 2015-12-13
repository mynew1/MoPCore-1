/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "SpellScript.h"

/*######
## Quest 25134: Lazy Peons
## npc_lazy_peon
######*/

enum LazyPeonYells
{
    SAY_SPELL_HIT = -1000600 // Ow! OK, I''ll get back to work, $N!'
};

enum LazyPeon
{
    QUEST_LAZY_PEONS    = 25134,
    GO_LUMBERPILE       = 175784,
    SPELL_BUFF_SLEEP    = 17743,
    SPELL_AWAKEN_PEON   = 19938
};

class npc_lazy_peon : public CreatureScript
{
public:
    npc_lazy_peon() : CreatureScript("npc_lazy_peon") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_lazy_peonAI(creature);
    }

    struct npc_lazy_peonAI : public ScriptedAI
    {
        npc_lazy_peonAI(Creature* creature) : ScriptedAI(creature) {}

        uint64 PlayerGUID;

        uint32 RebuffTimer;
        bool work;

        void Reset()
        {
            PlayerGUID = 0;
            RebuffTimer = 0;
            work = false;
        }

        void MovementInform(uint32 /*type*/, uint32 id)
        {
            if (id == 1)
                work = true;
        }

        void SpellHit(Unit* caster, const SpellInfo* spell)
        {
            if (spell->Id == SPELL_AWAKEN_PEON && caster->GetTypeId() == TYPEID_PLAYER
                && CAST_PLR(caster)->GetQuestStatus(QUEST_LAZY_PEONS) == QUEST_STATUS_INCOMPLETE)
            {
                caster->ToPlayer()->KilledMonsterCredit(me->GetEntry(), me->GetGUID());
                DoScriptText(SAY_SPELL_HIT, me, caster);
                me->RemoveAllAuras();
                if (GameObject* Lumberpile = me->FindNearestGameObject(GO_LUMBERPILE, 20))
                    me->GetMotionMaster()->MovePoint(1, Lumberpile->GetPositionX()-1, Lumberpile->GetPositionY(), Lumberpile->GetPositionZ());
            }
        }

        void UpdateAI(const uint32 Diff)
        {
            if (work == true)
                me->HandleEmoteCommand(EMOTE_ONESHOT_WORK_CHOPWOOD);
            if (RebuffTimer <= Diff)
            {
                DoCast(me, SPELL_BUFF_SLEEP);
                RebuffTimer = 300000; //Rebuff agian in 5 minutes
            }
            else
                RebuffTimer -= Diff;
            if (!UpdateVictim())
                return;
            DoMeleeAttackIfReady();
        }
    };
};

enum VoodooSpells
{
    SPELL_BREW      = 16712, // Special Brew
    SPELL_GHOSTLY   = 16713, // Ghostly
    SPELL_HEX1      = 16707, // Hex
    SPELL_HEX2      = 16708, // Hex
    SPELL_HEX3      = 16709, // Hex
    SPELL_GROW      = 16711, // Grow
    SPELL_LAUNCH    = 16716, // Launch (Whee!)
};

// 17009
class spell_voodoo : public SpellScriptLoader
{
    public:
        spell_voodoo() : SpellScriptLoader("spell_voodoo") {}

        class spell_voodoo_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_voodoo_SpellScript);

            bool Validate(SpellInfo const* /*spell*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_BREW) || !sSpellMgr->GetSpellInfo(SPELL_GHOSTLY) ||
                    !sSpellMgr->GetSpellInfo(SPELL_HEX1) || !sSpellMgr->GetSpellInfo(SPELL_HEX2) ||
                    !sSpellMgr->GetSpellInfo(SPELL_HEX3) || !sSpellMgr->GetSpellInfo(SPELL_GROW) ||
                    !sSpellMgr->GetSpellInfo(SPELL_LAUNCH))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                uint32 spellid = RAND(SPELL_BREW, SPELL_GHOSTLY, RAND(SPELL_HEX1, SPELL_HEX2, SPELL_HEX3), SPELL_GROW, SPELL_LAUNCH);
                if (Unit* target = GetHitUnit())
                    GetCaster()->CastSpell(target, spellid, false);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_voodoo_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_voodoo_SpellScript();
        }
};
enum BbladeCultist
{
	SPELL_INCINER = 79938,
	SPELL_SUMMIMP = 11939,
	SPELL_IMMOL = 11962,

	SPELL_FELBLOOD = 80174,
	SPELL_INFUSED = 84325,

	SAY_FLEE = 0 // 15%
};

// Burning Blade Cultist - 3199.
class npc_bblade_cultist : public CreatureScript
{
public:
	npc_bblade_cultist() : CreatureScript("npc_bblade_cultist") { }

	struct npc_bblade_cultistAI : public ScriptedAI
	{
		npc_bblade_cultistAI(Creature* creature) : ScriptedAI(creature) { }

		uint32 IncinerateTimer;
		uint32 ImmolateTimer;
		bool Flee;

		void Reset()
		{
			IncinerateTimer = urand(1000, 4000);
			ImmolateTimer = urand(5500, 9500);
			DoCast(me, SPELL_SUMMIMP);
			Flee = false;
		}

		void EnterEvadeMode()
		{
			me->RemoveAllAuras();
			Reset();
			me->DeleteThreatList();
			me->CombatStop(false);
			me->GetMotionMaster()->MoveTargetedHome();
		}

		void JustDied(Unit* killer)
		{
			if (killer->HasAura(SPELL_FELBLOOD))
			{
				if (AuraPtr aura = killer->GetAura(SPELL_FELBLOOD))
				{
					if (aura->GetStackAmount() >= 5)
					{
						killer->RemoveAurasDueToSpell(SPELL_FELBLOOD);
						me->AddAura(SPELL_INFUSED, killer);
					}
					else me->AddAura(SPELL_FELBLOOD, killer);
				}
			}
			else me->AddAura(SPELL_FELBLOOD, killer);
		}

		void UpdateAI(uint32 const diff)
		{
			if (!UpdateVictim() || me->HasUnitState(UNIT_STATE_CASTING))
				return;

			if (!Flee && HealthBelowPct(16))
			{
				Talk(SAY_FLEE);
				me->DoFleeToGetAssistance();
				IncinerateTimer = 10000;
				Flee = true;
			}

			if (IncinerateTimer <= diff)
			{
				DoCast(me->getVictim(), SPELL_INCINER);
				IncinerateTimer = urand(3000, 6000);
			}
			else IncinerateTimer -= diff;

			if (ImmolateTimer <= diff)
			{
				DoCast(me->getVictim(), SPELL_IMMOL);
				ImmolateTimer = urand(17000, 21000);
			}
			else ImmolateTimer -= diff;

			DoMeleeAttackIfReady();
		}
	};

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_bblade_cultistAI(creature);
	}
};

enum Watershed // quest 25187 item 52514 spell 73817 http://www.youtube.com/watch?v=J501FKs1CgE
{
	// Vehicles, on boarding give credit and eject back to place.
	WATERSHED_RAGARRAN = 39320,
	WATERSHED_TEKLA = 39345,
	WATERSHED_MISHA = 39346,
	WATERSHED_ZENTAJI = 39347,
};

void AddSC_durotar()
{
    new npc_lazy_peon();
    new spell_voodoo();
	new npc_bblade_cultist();
}
