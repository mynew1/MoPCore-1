/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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

/* ScriptData
SDName: Ironforge
SD%Complete: 0
SDComment:
SDCategory: Ironforge
EndScriptData */

/* ContentData
EndContentData */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"


// COLDRIDGE VALLEY

enum Coldridge
{
	SPELL_SHOOT = 3018,
	SPELL_CLUB = 69851,
	SPELL_CONSTRICTION_AURA = 77311,
	SPELL_THROW = 38557,
	SPELL_GEARS = 75775,

	NPC_COLDRIDGE_DEFENDER = 37177,
	NPC_ROCKJAW_INVADER = 37070,
	NPC_ROCKJAW_GOON = 37073,
	NPC_JOREN_IRONSTOCK = 37081,

	NPC_WOUNDED_MOUNTAINEER = 37080,

	NPC_KHARANOS_MOUNTAIR_1 = 41237,

	NPC_KHARANOS_MOUNTAINER = 41181,
	NPC_KHARANOS_RIFLEMAN = 41182,

	NPC_FROSTMANE_SCOUT = 41175,
	NPC_FROSTMANE_SCAVENGER = 41146
};

class item_paint : public ItemScript
{
public:
	item_paint() : ItemScript("item_paint"){}

	bool OnUse(Player* player, Item* /*item*/, SpellCastTargets const& targets){
		if (Creature* c = player->FindNearestCreature(42291, 5.0f, true)){
			if (c->GetEntry() == 42291){
				if (player->GetQuestStatus(26342) == QUEST_STATUS_INCOMPLETE){
					Quest const* qInfo = sObjectMgr->GetQuestTemplate(26342);
					player->KilledMonsterCredit(42796);
					c->DisappearAndDie();
				}
			}
		}
		return true;
	}
};

class item_techno_granade : public ItemScript
{
public:
	item_techno_granade() : ItemScript("item_techno_granade"){}

	bool OnUse(Player* player, Item* /*item*/, SpellCastTargets const& /*targets*/){
		player->CastSpell(player->getVictim(), 79751);
		return true;
	}
};

class npc_repaired_mechano_tank : public CreatureScript
{
public:
	npc_repaired_mechano_tank() : CreatureScript("npc_repaired_mechano_tank") { }

	struct npc_repaired_mechano_tankAI : ScriptedAI
	{
		npc_repaired_mechano_tankAI(Creature* creature) : ScriptedAI(creature)
		{
			SetCombatMovement(false);
		}

		void SpellHit(Unit* caster, SpellInfo const* spell) {
			//Get the player who cast the spell
			if (Player* p = caster->ToPlayer()){
				//Get the id of the spell 
				if (spell->Id == 79751){
					//if player has that quest incomplete
					if (p->GetQuestStatus(26333) == QUEST_STATUS_INCOMPLETE){
						//reward the monster
						Quest const* qInfo = sObjectMgr->GetQuestTemplate(26333);
						if (qInfo)
						{
							p->KilledMonsterCredit(42224);
							me->DisappearAndDie();
						}
					}
				}
			}
		}

	};

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_repaired_mechano_tankAI(creature);
	}
};

class go_makeshift_cage : public GameObjectScript
{
public:
	go_makeshift_cage() : GameObjectScript("go_makeshift_cage") { }

	bool OnGossipHello(Player* player, GameObject* go)
	{
		if (player->GetQuestStatus(26284) == QUEST_STATUS_INCOMPLETE){
			Quest const* qInfo = sObjectMgr->GetQuestTemplate(26284);
			player->KilledMonsterCredit(42645);
			go->Delete();
		}
		return false;
	}
};

class go_detonator : public GameObjectScript
{
public:
	go_detonator() : GameObjectScript("go_detonator") { }

	bool OnGossipHello(Player* player, GameObject* go)
	{
		if (player->GetQuestStatus(26318) == QUEST_STATUS_INCOMPLETE){
			Quest const* qInfo = sObjectMgr->GetQuestTemplate(26284);
			player->KilledMonsterCredit(-204042);
			go->Delete();
		}
		return false;
	}
};

class item_orbit : public ItemScript
{
public:
	item_orbit() : ItemScript("item_orbit"){}

	bool OnUse(Player* player, Item* /*item*/, SpellCastTargets const& targets){
		if (Creature* c = player->FindNearestCreature(42839, 10.0f, true)){
			if (player->GetQuestStatus(26364) == QUEST_STATUS_INCOMPLETE){
				Quest const* qInfo = sObjectMgr->GetQuestTemplate(26364);
				player->KilledMonsterCredit(42860);
				c->DisappearAndDie();
			}
		}
		return true;
	}
};

/*######
## npc_frostmane_troll
######*/

class npc_frostmane_troll : public CreatureScript
{
public:
	npc_frostmane_troll() : CreatureScript("npc_frostmane_troll") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_frostmane_trollAI(creature);
	}

	struct npc_frostmane_trollAI : public ScriptedAI
	{
		npc_frostmane_trollAI(Creature* creature) : ScriptedAI(creature)  { }

		uint32 DamageCount;
		uint32 AttackTimer;
		bool isMovingHome;

		void Reset()
		{
			if (me->GetEmoteState() != EMOTE_ONESHOT_NONE)
				me->SetReactState(REACT_PASSIVE);

			DamageCount = 0;
			AttackTimer = 1000;
			isMovingHome = false;
		}

		void EnterEvadeMode()
		{
			Reset();
			me->DeleteThreatList();
			me->CombatStop(false);
			me->GetMotionMaster()->MoveTargetedHome();
			isMovingHome = true;
		}

		void JustReachedHome()
		{
			isMovingHome = false;
		}

		void DamageDealt(Unit* target, uint32& damage, DamageEffectType damageType)
		{
			if (target->GetEntry() == NPC_KHARANOS_MOUNTAINER || target->GetEntry() == NPC_KHARANOS_RIFLEMAN)
				++DamageCount;

			if (DamageCount >= 2)
			{
				if ((target->GetEntry() == NPC_KHARANOS_MOUNTAINER || target->GetEntry() == NPC_KHARANOS_RIFLEMAN) && target->HealthBelowPct(100))
					damage = 0;
				else
					DamageCount = 0;
			}
		}

		void DamageTaken(Unit* who, uint32 &uiDamage)
		{
			if (who->GetTypeId() == TYPEID_PLAYER || who->isPet())
			{
				if (me->GetEntry() == NPC_FROSTMANE_SCAVENGER)
					if (me->GetEmoteState() != EMOTE_ONESHOT_NONE)
					{
						me->HandleEmote(EMOTE_ONESHOT_NONE);
						me->SetReactState(REACT_AGGRESSIVE);
					}

				me->getThreatManager().resetAllAggro();
				me->AddThreat(who, 100.0f);
				me->AI()->AttackStart(who);
				DamageCount = 0;

				if (me->GetEntry() == NPC_FROSTMANE_SCOUT)
				{
					switch (urand(0, 3))
					{
					case 0: me->MonsterSay("I gonna make you into mojo!", LANG_UNIVERSAL, 0); break;
					case 1: me->MonsterSay("Killing you be easy.", LANG_UNIVERSAL, 0); break;
					case 2: me->MonsterSay("My weapon be thirsty!", LANG_UNIVERSAL, 0); break;
					case 3: me->MonsterSay("You be dead soon!", LANG_UNIVERSAL, 0); break;
					default: break;
					}

					if (!me->IsWithinDist(me->getVictim(), ATTACK_DISTANCE))
						DoCast(me->getVictim(), SPELL_THROW);
				}
				else if (me->GetEntry() == NPC_FROSTMANE_SCAVENGER)
				{
					if (!me->IsWithinDist(me->getVictim(), ATTACK_DISTANCE))
						DoCast(me->getVictim(), SPELL_GEARS);
				}
			}
		}

		void UpdateAI(uint32 const diff)
		{
			if (AttackTimer <= diff)
			{
				if (!me->isInCombat() && !isMovingHome)
				{
					if (me->GetEmoteState() == EMOTE_ONESHOT_NONE)
					{
						if (me->FindNearestCreature(NPC_KHARANOS_MOUNTAINER, 7.0f))
						{
							if (Creature* kharanosM = me->FindNearestCreature(NPC_KHARANOS_MOUNTAINER, 7.0f))
								if (!kharanosM->FindNearestGameObject(203193, 3.0f))
									me->AI()->AttackStart(kharanosM);
						}
						else
						{
							if (me->FindNearestCreature(NPC_KHARANOS_RIFLEMAN, 7.0f))
							{
								if (Creature* kharanosR = me->FindNearestCreature(NPC_KHARANOS_RIFLEMAN, 7.0f))
									me->AI()->AttackStart(kharanosR);
							}
						}
						AttackTimer = 2000;
					}
					else
					{
						me->SetReactState(REACT_PASSIVE);
						AttackTimer = -1;
					}
				}
			}
			else AttackTimer -= diff;

			if (!UpdateVictim())
				return;

			DoMeleeAttackIfReady();
		}
	};
};

/*######
## npc_kharanos_mountaineer
######*/

class npc_kharanos_mountaineer : public CreatureScript
{
public:
	npc_kharanos_mountaineer() : CreatureScript("npc_kharanos_mountaineer") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_kharanos_mountaineerAI(creature);
	}

	struct npc_kharanos_mountaineerAI : public ScriptedAI
	{
		npc_kharanos_mountaineerAI(Creature* creature) : ScriptedAI(creature) { }

		uint32 uiSayCombatTimer;
		uint32 uiMortarShoot;
		uint32 uiShootTimer;
		uint32 DamageCount;

		void Reset()
		{
			if (me->HasUnitState(UNIT_STATE_ROOT))
				me->ClearUnitState(UNIT_STATE_ROOT);

			if (me->FindNearestGameObject(203193, 3.0f))
				me->SetReactState(REACT_PASSIVE);

			uiSayCombatTimer = urand(10000, 190000);
			uiMortarShoot = urand(2000, 3000);
			uiShootTimer = urand(2000, 3000);
			DamageCount = 0;
		}

		void EnterCombat(Unit* /*who*/)
		{
			if (!me->HasUnitState(UNIT_STATE_ROOT))
				me->AddUnitState(UNIT_STATE_ROOT);
		}

		void DamageTaken(Unit* who, uint32 &uiDamage)
		{
			if (who->GetTypeId() == TYPEID_UNIT && (who->GetEntry() == NPC_FROSTMANE_SCOUT || who->GetEntry() == NPC_FROSTMANE_SCAVENGER) && !me->isInCombat() && !me->FindNearestGameObject(203193, 3.0f))
				me->AI()->AttackStart(who);
		}

		void DamageDealt(Unit* target, uint32& damage, DamageEffectType damageType)
		{
			if (target->GetEntry() == NPC_FROSTMANE_SCOUT || target->GetEntry() == NPC_FROSTMANE_SCAVENGER)
				++DamageCount;

			if (DamageCount >= 2)
			{
				if ((target->GetEntry() == NPC_FROSTMANE_SCOUT || target->GetEntry() == NPC_FROSTMANE_SCAVENGER) && target->HealthBelowPct(100))
					damage = 0;
				else
					DamageCount = 0;
			}
		}

		void EnterEvadeMode()
		{
			Reset();
			me->DeleteThreatList();
			me->CombatStop(false);
			me->GetMotionMaster()->MoveTargetedHome();
		}

		void UpdateAI(uint32 const diff)
		{
			if (uiMortarShoot <= diff)
			{
				if (me->GetEntry() == NPC_KHARANOS_MOUNTAINER)
				{
					if (GameObject* Mortar = me->FindNearestGameObject(203193, 3.0f))
					{
						std::list<Creature*> creatures;
						GetCreatureListWithEntryInGrid(creatures, me, NPC_FROSTMANE_SCOUT, 60.0f);
						if (!creatures.empty())
							for (std::list<Creature*>::iterator iter = creatures.begin(); iter != creatures.end(); ++iter)
								if (!me->IsWithinCombatRange((*iter), 10.0f))
								{
									Mortar->CastSpell((*iter), 77091); // Mortar Fire
									break;
								}
						uiMortarShoot = urand(6000, 14000);
					}
					else uiMortarShoot = -1;
				}
			}
			else uiMortarShoot -= diff;

			if (!UpdateVictim())
				return;

			if (me->HealthBelowPct(100))
			{
				if (uiSayCombatTimer <= diff)
				{
					switch (urand(0, 2))
					{
					case 0: me->MonsterSay("Take 'em down!", LANG_UNIVERSAL, 0); break;
					case 1: me->MonsterSay("Time ta' crack some troll skulls!", LANG_UNIVERSAL, 0); break;
					case 2: me->MonsterSay("Push 'em back!", LANG_UNIVERSAL, 0); break;
					default: break;
					}
					uiSayCombatTimer = urand(10000, 250000);
				}
				else uiSayCombatTimer -= diff;
			}

			if (uiShootTimer <= diff)
			{
				if (me->GetEntry() == NPC_KHARANOS_RIFLEMAN)
				{
					if (!me->IsWithinDist(me->getVictim(), ATTACK_DISTANCE))
						DoCast(me->getVictim(), SPELL_SHOOT);
					uiShootTimer = urand(2000, 3000);
				}
				else uiShootTimer = -1;
			}
			else uiShootTimer -= diff;

			DoMeleeAttackIfReady();
		}
	};
};

/*######
## npc_constriction_totem
######*/

class npc_constriction_totem : public CreatureScript
{
public:
	npc_constriction_totem() : CreatureScript("npc_constriction_totem") { }

	struct npc_constriction_totemAI : public ScriptedAI
	{
		npc_constriction_totemAI(Creature* creature) : ScriptedAI(creature) { }

		bool MountaineerFreed;
		Creature* MyMountaineer;
		uint32 CastTimer;

		void Reset()
		{
			if (Creature* Mountaineer = me->FindNearestCreature(NPC_KHARANOS_MOUNTAIR_1, 8.0f, true))
			{
				if (!Mountaineer->HasAura(SPELL_CONSTRICTION_AURA))
				{
					MyMountaineer = Mountaineer;
					me->AddAura(SPELL_CONSTRICTION_AURA, Mountaineer);
					Mountaineer->SetReactState(REACT_PASSIVE);
				}
			}
			else MyMountaineer = NULL;
			MountaineerFreed = false;
			CastTimer = 2000;
		}

		void SpellHit(Unit* caster, const SpellInfo* spell)
		{
			if (spell->Id == 77314 && !MountaineerFreed && MyMountaineer)
			{
				MyMountaineer->RemoveAurasDueToSpell(SPELL_CONSTRICTION_AURA);
				MyMountaineer->MonsterSay("Thank you for freeing me!", LANG_UNIVERSAL, 0);
				float x, y, z;
				MyMountaineer->GetClosePoint(x, y, z, MyMountaineer->GetObjectSize() / 3, 5.0f);
				MyMountaineer->GetMotionMaster()->MovePoint(1, x, y, z);
				MyMountaineer->DespawnOrUnsummon(5000);
				me->DespawnOrUnsummon(1000);
				MountaineerFreed = true;
			}
		}

		void UpdateAI(uint32 const diff)
		{
			if (CastTimer <= diff)
			{
				if (!MyMountaineer && !MountaineerFreed)
				{
					if (Creature* Mountaineer = me->FindNearestCreature(NPC_KHARANOS_MOUNTAIR_1, 8.0f, true))
					{
						if (!Mountaineer->HasAura(SPELL_CONSTRICTION_AURA))
						{
							MyMountaineer = Mountaineer;
							me->AddAura(SPELL_CONSTRICTION_AURA, Mountaineer);
							Mountaineer->SetReactState(REACT_PASSIVE);
							CastTimer = -1;
						}
						else CastTimer = 2000;
					}
					else CastTimer = 2000;
				}
				else CastTimer = -1;
			}
			else CastTimer -= diff;
		}
	};

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_constriction_totemAI(creature);
	}
};


/*######
## npc_coldridge_defender
######*/

class npc_coldridge_defender : public CreatureScript
{
public:
	npc_coldridge_defender() : CreatureScript("npc_coldridge_defender") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_coldridge_defenderAI(creature);
	}

	struct npc_coldridge_defenderAI : public ScriptedAI
	{
		npc_coldridge_defenderAI(Creature* creature) : ScriptedAI(creature) { }

		uint32 uiSayNormalTimer;
		uint32 uiSayCombatTimer;
		uint32 uiShootTimer;
		uint32 DamageCount;

		void Reset()
		{
			if (me->HasUnitState(UNIT_STATE_ROOT))
				me->ClearUnitState(UNIT_STATE_ROOT);

			uiSayNormalTimer = urand(5000, 155000);
			uiSayCombatTimer = urand(10000, 160000);
			uiShootTimer = urand(2000, 4000);

			DamageCount = 0;
		}

		void EnterCombat(Unit* /*who*/)
		{
			if (!me->HasUnitState(UNIT_STATE_ROOT))
				me->AddUnitState(UNIT_STATE_ROOT);
		}

		void DamageTaken(Unit* who, uint32 &uiDamage)
		{
			if (who->GetTypeId() == TYPEID_UNIT && (who->GetEntry() == NPC_ROCKJAW_INVADER || who->GetEntry() == NPC_ROCKJAW_GOON) && !me->isInCombat())
				me->AI()->AttackStart(who);
		}

		void DamageDealt(Unit* target, uint32& damage, DamageEffectType damageType)
		{
			if (target->GetEntry() == NPC_ROCKJAW_INVADER || target->GetEntry() == NPC_ROCKJAW_GOON)
				++DamageCount;

			if (DamageCount >= 2)
			{
				if ((target->GetEntry() == NPC_ROCKJAW_INVADER || target->GetEntry() == NPC_ROCKJAW_GOON) && target->HealthBelowPct(100))
					damage = 0;
				else
					DamageCount = 0;
			}
		}

		void EnterEvadeMode()
		{
			Reset();
			me->DeleteThreatList();
			me->CombatStop(false);
			me->GetMotionMaster()->MoveTargetedHome();
		}

		void UpdateAI(uint32 const diff)
		{
			if (uiSayNormalTimer <= diff && me->GetPositionX() < -6208.0f)
			{
				switch (urand(0, 3))
				{
				case 0: me->MonsterSay("Where do all 'em troggs keep coming from?", LANG_UNIVERSAL, 0); break;
				case 1: me->MonsterSay("They won't pass through all'o us!", LANG_UNIVERSAL, 0); break;
				case 2: me->MonsterSay("Look at 'em large ones!", LANG_UNIVERSAL, 0); break;
				case 3: me->MonsterSay("'Em cursed troggs just keep coming!", LANG_UNIVERSAL, 0); break;
				default: break;
				}
				uiSayNormalTimer = urand(20000, 180000);
			}
			else uiSayNormalTimer -= diff;

			if (!UpdateVictim())
				return;

			if (me->HealthBelowPct(100))
			{
				if (uiSayCombatTimer <= diff)
				{
					switch (urand(0, 3))
					{
					case 0: me->MonsterSay("Blast 'em!", LANG_UNIVERSAL, 0); break;
					case 1: me->MonsterSay("Dese troggs don't know when ta' stop!", LANG_UNIVERSAL, 0); break;
					case 2: me->MonsterSay("Yer' time is now!", LANG_UNIVERSAL, 0); break;
					case 3: me->MonsterSay("Hold yer' lines, gents!", LANG_UNIVERSAL, 0); break;
					default: break;
					}
					uiSayCombatTimer = urand(10000, 170000);
				}
				else uiSayCombatTimer -= diff;
			}

			if (uiShootTimer <= diff)
			{
				if (!me->IsWithinDist(me->getVictim(), ATTACK_DISTANCE))
					DoCast(me->getVictim(), SPELL_SHOOT);
				uiShootTimer = urand(2000, 3000);
			}
			else uiShootTimer -= diff;

			DoMeleeAttackIfReady();
		}
	};
};

/*######
## npc_rockjaw_invader + goon
######*/

class npc_rockjaw_invader : public CreatureScript
{
public:
	npc_rockjaw_invader() : CreatureScript("npc_rockjaw_invader") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_rockjaw_invaderAI(creature);
	}

	struct npc_rockjaw_invaderAI : public ScriptedAI
	{
		npc_rockjaw_invaderAI(Creature* creature) : ScriptedAI(creature)  { }

		uint32 DamageCount;
		uint32 AttackTimer;
		uint32 ClubTimer;
		bool isMovingHome, said;

		void Reset()
		{
			DamageCount = 0;
			AttackTimer = 1000;
			ClubTimer = -1;
			isMovingHome = false;
			said = false;
		}

		void EnterEvadeMode()
		{
			Reset();
			me->DeleteThreatList();
			me->CombatStop(false);
			me->GetMotionMaster()->MoveTargetedHome();
			isMovingHome = true;
		}

		void JustReachedHome()
		{
			isMovingHome = false;
		}

		void DamageDealt(Unit* target, uint32& damage, DamageEffectType damageType)
		{
			if (target->GetEntry() == NPC_COLDRIDGE_DEFENDER)
				++DamageCount;

			if (DamageCount >= 2)
			{
				if (target->GetEntry() == NPC_COLDRIDGE_DEFENDER && target->HealthBelowPct(100))
					damage = 0;
				else
					DamageCount = 0;
			}
		}

		void DamageTaken(Unit* who, uint32 &uiDamage)
		{
			if (who->GetTypeId() == TYPEID_PLAYER || who->isPet())
			{
				me->getThreatManager().resetAllAggro();
				me->AddThreat(who, 100.0f);
				me->AI()->AttackStart(who);
				DamageCount = 0;

				if (me->GetEntry() == NPC_ROCKJAW_GOON && !said)
				{
					switch (urand(0, 2))
					{
					case 0: me->MonsterSay("Trogg cave all gone! You pay for this!!", LANG_UNIVERSAL, 0); break;
					case 1: me->MonsterSay("Why you break our home?!", LANG_UNIVERSAL, 0); break;
					case 2: me->MonsterSay("You break our cave, I break your skull!", LANG_UNIVERSAL, 0); break;
					default: break;
					}
					ClubTimer = 3000;
					said = true;
				}
			}
		}

		void UpdateAI(uint32 const diff)
		{
			if (AttackTimer <= diff)
			{
				if (!me->isInCombat() && !isMovingHome && me->FindNearestCreature(NPC_COLDRIDGE_DEFENDER, 7.0f))
					if (Creature* defender = me->FindNearestCreature(NPC_COLDRIDGE_DEFENDER, 7.0f))
						me->AI()->AttackStart(defender);
				AttackTimer = 2000;
			}
			else AttackTimer -= diff;

			if (ClubTimer <= diff)
			{
				if (Unit* target = me->getVictim())
					DoCast(target, SPELL_CLUB);
				ClubTimer = urand(5000, 8000);
			}
			else ClubTimer -= diff;

			if (!UpdateVictim())
				return;

			DoMeleeAttackIfReady();
		}
	};
};

/*######
## npc_wounded_mountaineer
######*/

class npc_wounded_mountaineer : public CreatureScript
{
public:
	npc_wounded_mountaineer() : CreatureScript("npc_wounded_mountaineer") { }

	struct npc_wounded_mountaineerAI : public ScriptedAI
	{
		npc_wounded_mountaineerAI(Creature* creature) : ScriptedAI(creature) { }

		bool IsHealed;

		void Reset()
		{
			IsHealed = false;
		}

		void SpellHit(Unit* caster, const SpellInfo* spell)
		{
			if (spell->Id == 69855 && !IsHealed)
			{
				switch (urand(0, 3))
				{
				case 0: me->MonsterSay("Yer a lifesaver! For Ironforge!", LANG_UNIVERSAL, 0); break;
				case 1: me->MonsterSay("Thanks, lad! Feels good ta' be in one piece again.", LANG_UNIVERSAL, 0); break;
				case 2: me->MonsterSay("Ahh, much better. I owe ya one.", LANG_UNIVERSAL, 0); break;
				case 3: me->MonsterSay("Thanks! I can make it from here.", LANG_UNIVERSAL, 0); break;
				default: break;
				}
				me->SetStandState(UNIT_STAND_STATE_STAND);
				me->HandleEmote(EMOTE_ONESHOT_BOW);
				me->DespawnOrUnsummon(3000);
				IsHealed = true;
			}
		}
	};

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_wounded_mountaineerAI(creature);
	}
};

/*######
## npc_troll_soothsayer
######*/

class npc_troll_soothsayer : public CreatureScript
{
public:
	npc_troll_soothsayer() : CreatureScript("npc_troll_soothsayer") { }

	struct npc_troll_soothsayerAI : public ScriptedAI
	{
		npc_troll_soothsayerAI(Creature* creature) : ScriptedAI(creature) { }

		bool saidText;
		uint32 ResetTimer;

		void Reset()
		{
			saidText = false;
			ResetTimer = -1;
		}

		void MoveInLineOfSight(Unit* who)
		{
			if (!saidText && who->GetTypeId() == TYPEID_PLAYER && who->IsWithinDistInMap(me, 10.0f))
				if (who->ToPlayer()->GetQuestStatus(24489) == QUEST_STATUS_INCOMPLETE)
				{
					switch (me->GetEntry())
					{
					case 37108:  // Soothsayer Shi'kala
						me->MonsterSay("Da spirits be angry with us. I don' know why the spirits be rejectin' us so. Don' worry, child. Grik'nir gonna help us get through this.", LANG_UNIVERSAL, 0); break;
					case 37173: // Soothsayer Rikkari
						me->MonsterSay("What we gon' do now, you ask? We wait. Grik'nir says he gon' talk to the elemental, get it to fight on our side. Soon enough we take over dis valley. Soon enough.", LANG_UNIVERSAL, 0); break;
					case 37174: // Soothsayer Mirim'koa
						me->MonsterSay("Our land be a land of ice an' snow. But beneath the earth, child, there always be fire. De spirit come from deep down to talk with Grik'nir.", LANG_UNIVERSAL, 0); break;
					default: break;
					}
					if (Player* player = who->ToPlayer())
						player->KilledMonsterCredit(me->GetEntry(), 0);
					saidText = true;
					ResetTimer = 30000;
				}
		}

		void UpdateAI(uint32 const diff)
		{
			if (ResetTimer <= diff)
			{
				saidText = false;
				ResetTimer = -1;
			}
			else ResetTimer -= diff;
		}
	};

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_troll_soothsayerAI(creature);
	}
};

/*######
## npc_wounded_militia
######*/

class npc_wounded_militia : public CreatureScript
{
public:
	npc_wounded_militia() : CreatureScript("npc_wounded_militia") { }

	struct npc_wounded_militiaAI : public ScriptedAI
	{
		npc_wounded_militiaAI(Creature* creature) : ScriptedAI(creature) { }

		bool IsHealed;

		void Reset()
		{
			IsHealed = false;
		}

		void SpellHit(Unit* caster, const SpellInfo* spell)
		{
			if (spell->Id == 2061 && !IsHealed)
			{
				switch (urand(0, 2))
				{
				case 0: me->MonsterSay("Right on! Could use a beer, now!", LANG_UNIVERSAL, 0); break;
				case 1: me->MonsterSay("Thanks, lad!", LANG_UNIVERSAL, 0); break;
				case 2: me->MonsterSay("Wow! Suddenly feelin' me power comin' back!", LANG_UNIVERSAL, 0); break;
				default: break;
				}
				me->SetStandState(UNIT_STAND_STATE_STAND);
				me->HandleEmote(EMOTE_ONESHOT_CHEER);
				me->DespawnOrUnsummon(4000);
				IsHealed = true;
			}
		}
	};

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_wounded_militiaAI(creature);
	}
};

/*######
## npc_mountaineer_watch
######*/

class npc_mountaineer_watch : public CreatureScript
{
public:
	npc_mountaineer_watch() : CreatureScript("npc_mountaineer_watch") { }

	bool OnGossipHello(Player* player, Creature* creature)
	{
		if (player->GetQuestStatus(313) == QUEST_STATUS_INCOMPLETE)
			player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Captain Tharran wants you to deploy your remote observation bots and withdraw to Kharanos.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
		player->SEND_GOSSIP_MENU(creature->GetEntry(), creature->GetGUID());
		return true;
	}

	bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
	{
		player->PlayerTalkClass->ClearMenus();

		if (action == GOSSIP_ACTION_INFO_DEF + 1)
		{
			switch (creature->GetEntry())
			{
			case 40991: // Mountaineer Dunstan
				creature->MonsterSay("Tell Captain Tharran that I'll be back in Kharanos as soon as I've verified that the bot is working correctly.", LANG_UNIVERSAL, 0);
				player->CLOSE_GOSSIP_MENU();
				break;
			case 40994: // Mountaineer Lewin
				creature->MonsterSay("I can't wait to get out there and help in the fight against those trolls.", LANG_UNIVERSAL, 0);
				player->CLOSE_GOSSIP_MENU();
				break;
			case 41056: // Mountaineer Valgrum
				creature->MonsterSay("We've been fighting nonstop since the cataclysm. It'll be nice to get a rest, if a brief one.", LANG_UNIVERSAL, 0);
				player->CLOSE_GOSSIP_MENU();
				break;
			}

			float x, y, z;
			creature->GetClosePoint(x, y, z, creature->GetObjectSize() / 3, 5.0f);
			if (Creature* bot = creature->SummonCreature(41052, x, y, z, 0, TEMPSUMMON_MANUAL_DESPAWN)) // Summon Remote Observation Bot.
			{
				bot->GetMotionMaster()->MoveRandom(10.0f);
				bot->DespawnOrUnsummon(10000);
			}

			player->KilledMonsterCredit(creature->GetEntry(), 0);
			creature->DespawnOrUnsummon(6000);
		}

		return true;
	}
};

void AddSC_ironforge()
{
	new item_paint();
	new item_techno_granade();
	new npc_repaired_mechano_tank();
	new go_makeshift_cage();
	new item_orbit();
	new go_detonator();
	new npc_coldridge_defender();
	new npc_rockjaw_invader();
	new npc_wounded_mountaineer();
	new npc_troll_soothsayer();
	new npc_wounded_militia();
	new npc_mountaineer_watch();
	new npc_constriction_totem();
	new npc_kharanos_mountaineer();
	new npc_frostmane_troll();
}
