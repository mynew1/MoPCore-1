

#include "ScriptPCH.h"
#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"
#include "MapManager.h"
#include "Spell.h"
#include "Vehicle.h"
#include "Cell.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "CreatureTextMgr.h"
#include "MoveSplineInit.h"
#include "Weather.h"

#include "heart_of_fear.h"

/*
During the fight, you will have to fight Mel'jarak, as well as 9 adds:
- 3 Sra-thik Amber-Trappers,
- 3 Kor'thic Elite Blademasters,
- 3 Zar'thik Battle-Menders.
The adds will all become active as soon as you engage the boss.

The three mobs in a group are tied together:
- Adds of the same kind share a health pool and have the same abilities, so they die together.
- If all three fall, Mel'jarak gains [Recklessness] and summons another group of the same type 50 seconds later.

!Note: ~ Normal - he becomes reckless for the remainder of the fight. Damage dealt +50%, taken +33%. Stacks.
~ Heroic - he becomes reckless for 30 sec. Damage dealt +100%, taken +600%.

Before the encounter begins, four players should interact with the weapon racks left along the wall to gain a bonus ability, Impaling Spear.
This can be used to crowd control one mantid for 50 seconds. The spear can be reapplied when desired, but Mel'jarak will only let so many of his warriors be cc'ed at a time.

- [Watchful Eye] : Adds which are crowd-controlled:
1 ~ when 9 adds are alive, you can crowd-control 4 adds;
2 ~ when 6 adds are alive, you can crowd-control 2 adds;
3 ~ when 3 adds are alive, you can crowd-control 0 adds.

- On Normal, [Watchful Eye] debuff changes depending on adds up/killed.
- On Heroic, Wind Lord Mel'jarak only uses basic [Watchful Eye] debuff and only dispels crowd control effects if players incapacitate more than three warriors.
*/

enum Yells
{
	/*** Boss ***/
	SAY_INTRO = 0, // Your defiance of the empress ends here!

	SAY_AGGRO = 1, // All of Pandaria will fall beneath the Wings of the Empress!
	SAY_SLAY = 2, // 0 - The Empress commands it!; 1 - Pitiful.
	SAY_DEATH = 3, // Empress... I have... failed you...

	SAY_WHIRLING_BLADES = 4, // My blade never misses its mark!
	SAY_RAIN_OF_BLADES = 5, // The skies belong to the Empress!

	SAY_WATCHFUL_EYE = 6, // 0 - You fight like cowards!; 1 - Aid me brethren!

	SAY_ADD_GROUP_DIES = 7, // 0 - To die for the empress is an honor!; 1 - The wings of the empress cannot fail!; 2 - You will pay for your transgressions!
	SAY_SUMMON_REINFORCE = 8,  // 0 - The Kor'thik have no equal in combat!; 1 - The mighty Zar'thik bend the wind to their will!; 2 - The Sra'thik command the elements of this land, leaving their foes imprisoned for all time!

	ANN_WHIRLING_BLADES = 9, // Wind Lord Mel'jarak begins to cast [Whirling Blades]!
	ANN_RAIN_OF_BLADES = 10,// Wind Lord Mel'jarak begins to cast [Rain of Blades]!
	ANN_REINFORCEMENTS = 11 // Wind Lord Mel'jarak calls for reinforcements!
};

enum Spells
{
	/*** Boss ***/
	SPELL_WHIRLING_BLADE = 121896, // Triggers 121898 damage every 0.25s and 121897, 122083 dummy location spells (visuals?).
	SPELL_RAIN_OF_BLADES = 122406, // Triggers 122407 damage spell every 0.5s.
	SPELL_WIND_BOMB = 131813, // Triggers 131814 bomb summon npc 67053.

	SPELL_WATCHFUL_EYE_1 = 125933, // Starting Normal and always Heroic aura - 9 adds alive, can control 4 / 3 on Heroic.
	SPELL_WATCHFUL_EYE_2 = 125935, // 6 adds alive, can control 2.
	SPELL_WATCHFUL_EYE_3 = 125936, // 3 adds alive, can control 0.

	SPELL_RECKLESNESS_N = 122354, // Normal stackable version.
	SPELL_RECKLESNESS_H = 125873, // Heroic 30 sec version.

	SPELL_BERSERK_MELJARAK = 120207, // 480 seconds or 8 mins Enrage.

	/*** Misc / Impaling Spear ***/
	SPELL_IMP_SPEAR_ABIL = 122220, // Aura player receives when clicking on the racks. Gives Action Bar with button 1 to throw spear.
	SPELL_IMP_SPEAR = 122224, // Incapacitates an add for 50 seconds, damage breaks it.

	/*** Adds ***/
	// Wind Bomb
	SPELL_WIND_BOMB_THR_DMG = 131830, // Damage in 5 yards at throwing.
	SPELL_WIND_BOMB_ARM = 131835, // Visual bomb arm.
	SPELL_WIND_BOMB_EXPLODE = 131842, // If a player goes in 6 yards (on off checked with 131836).

	// The Swarm
	SPELL_FATE_OF_THE_KORT = 121774, // Share Damage spell for Kor'thik Elite Blademasters.
	SPELL_FATE_OF_THE_SRAT = 121802, // Share Damage spell for Sra'thik Amber-Trappers.
	SPELL_FATE_OF_THE_ZART = 121807, // Share Damage spell for Zar'thik Battle-Menders.

	// - Kor'thik Elite Blademaster
	SPELL_KORTHIK_STRIKE = 123962, // All 3 use this at once on the same player.

	// - Sra'thik Amber-Trapper
	SPELL_AMBER_PRISON = 121876, // Initial cast, triggers 121881 after 3 seconds.
	SPELL_AMBER_PRISON_TRIG = 121881, // Triggers 121874 cast.
	SPELL_AMBER_PRISON_FC = 121874, // Triggers 121885 aura.
	SPELL_AMBER_PRISON_AURA = 121885, // Stun, root, visual etc. !ADD THIS: /* insert into spell_linked_spell values (121885, 129078, 1, 'Summon Amber Prison when hit by aura'); */
	SPELL_AMBER_PRISON_SUMM = 129078, // Summons 62531 Amber Prison npc.

	SPELL_RESIDUE = 122055, // Residue afflicts players who destroy Amber Prisons.

	SPELL_CORROSIVE_RESIN = 122064,

	// !-  Moving while afflicted by Corrosive Resin removes a stack of Corrosive Resin and creates a Corrosive Resin Pool at the player's location. -!
	SPELL_CORR_RESIN_POOL_S = 122123, // Summons Corrosive Resin Pool npc 67046.
	SPELL_CORR_RESIN_POOL_A = 129005, // Aura for Corrosive Resin Pool, triggers 122125 dmg each sec.

	// - Zar'thik Battle-Mender
	SPELL_MENDING = 122193, // Triggers a 200 yard dummy + heal spell 122147.
	SPELL_QUICKENING = 122149  // All Swarm gets boost 25% dmg + as.
};

enum Events
{
	/*** Boss ***/
	EVENT_WHIRLING_BLADE = 1, // 36 secs after pull
	EVENT_RAIN_OF_BLADES,         // 60 secs after pull
	EVENT_WIND_BOMB,

	EVENT_CHECK_ADD_CC_DEATH,     // Checks how many adds are dead / CC'ed.
	EVENT_SUMMON_REINFORCEMENTS,

	EVENT_BERSERK_MELJARAK,

	/*** Adds ***/
	// Wind Bomb
	EVENT_ARM,                    // 3 secs after spawn
	EVENT_CHECK_PLAYER,           // Check if needs to explode.

	// The Swarm

	// - Kor'thik Elite Blademaster
	EVENT_KORTHIK_STRIKE,         // 19s after pull.

	// - Sra'thik Amber-Trapper
	EVENT_AMBER_PRISON,
	EVENT_CORROSIVE_RESIN,

	// - Zar'thik Battle-Mender
	EVENT_MENDING,
	EVENT_QUICKENING
};

enum Adds
{
	NPC_KORTHIK_ELITE_BLADEMASTER = 62402,
	NPC_SRATHIK_AMBER_TRAPPER = 62405,
	NPC_ZARTHIK_BATTLE_MENDER = 62408
};
/*
// Mel'jarak add spawn points.
Position const AddSummonPositions[9] =
{
{ -2064.49f, 470.35f, 503.569f, 2.18f },
{ -2076.037, 457.40f, 503.569f, 2.18f },
{ 4749.9819f, 79.0285f, 96.380f, 6.03f }, // 3 NPC_KORTHIK_ELITE_BLADEMASTER.
{ 4768.8496f, 30.3054f, 89.410f, 1.53f },
{ 4786.0669f, 38.0198f, 94.897f, 1.99f },
{ 4749.9819f, 79.0285f, 96.380f, 6.03f }, // 3 NPC_SRATHIK_AMBER_TRAPPER.
{ 4768.8496f, 30.3054f, 89.410f, 1.53f },
{ 4786.0669f, 38.0198f, 94.897f, 1.99f },
{ 4749.9819f, 79.0285f, 96.380f, 6.03f }, // 3 NPC_ZARTHIK_BATTLE_MENDER.
};

// Mel'jarak reinforcements spawn points.
Position const ReinforcementsSummonPositions[3] =
{
{ 4768.8496f, 30.3054f, 89.410f, 1.53f },
{ 4786.0669f, 38.0198f, 94.897f, 1.99f },
{ 4749.9819f, 79.0285f, 96.380f, 6.03f },
};*/

class boss_wind_lord_meljarak : public CreatureScript
{
public:
	boss_wind_lord_meljarak() : CreatureScript("boss_wind_lord_meljarak") { }

	struct boss_wind_lord_meljarakAI : public BossAI
	{
		boss_wind_lord_meljarakAI(Creature* creature) : BossAI(creature, DATA_MELJARAK), summons(me)
		{
			instance = creature->GetInstanceScript();
			introDone = false;
			creature->AddAura(SPELL_WATCHFUL_EYE_1, creature);
		}

		InstanceScript* instance;
		SummonList summons;
		EventMap events;
		bool introDone, windBombScheduled, reinforcementsSummoned;

		void Reset()
		{
			events.Reset();
			summons.DespawnAll();

			windBombScheduled = false;
			reinforcementsSummoned = false;

			_Reset();

			// Summon the adds.
			SummonSwarmAdds();
		}

		void MoveInLineOfSight(Unit* who)
		{
			if (introDone || !who->IsWithinDistInMap(me, 40.0f))
				return;

			Talk(SAY_INTRO);
			introDone = true;
		}

		void EnterCombat(Unit* /*who*/)
		{
			Talk(SAY_AGGRO);

			// Special Adds Check event.
			events.ScheduleEvent(EVENT_CHECK_ADD_CC_DEATH, 1000);

			// Normal events.
			events.ScheduleEvent(EVENT_WHIRLING_BLADE, 36000);
			events.ScheduleEvent(EVENT_RAIN_OF_BLADES, 60000);
			events.ScheduleEvent(EVENT_BERSERK, 8 * MINUTE * IN_MILLISECONDS);

			// Elite Battlemasters event.
			events.ScheduleEvent(EVENT_KORTHIK_STRIKE, 19000);

			if (instance)
			{
				instance->SetData(DATA_MELJARAK, IN_PROGRESS);
				instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me); // Add
			}

			_EnterCombat();
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
			me->CombatStop(false);
			me->GetMotionMaster()->MoveTargetedHome();

			if (instance)
			{
				instance->SetData(DATA_MELJARAK, FAIL);
				instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me); // Remove
			}

			_EnterEvadeMode();
		}

		void JustReachedHome()
		{
			DoCast(me, SPELL_WATCHFUL_EYE_1);
		}

		void JustDied(Unit* /*killer*/)
		{
			Talk(SAY_DEATH);
			summons.DespawnAll();

			if (instance)
			{
				instance->SetData(DATA_MELJARAK, DONE);
				instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me); // Remove
			}

			_JustDied();
		}

		void JustSummoned(Creature* summon)
		{
			summons.Summon(summon);
			summon->setActive(true);

			if (me->isInCombat())
				summon->AI()->DoZoneInCombat();
		}

		void UpdateAI(uint32 const diff)
		{
			if (!UpdateVictim() || me->HasUnitState(UNIT_STATE_CASTING))
				return;

			if (me->HealthBelowPct(76) && !windBombScheduled)
			{
				events.ScheduleEvent(EVENT_WIND_BOMB, 6000);
				windBombScheduled = true;
			}

			events.Update(diff);

			while (uint32 eventId = events.ExecuteEvent())
			{
				switch (eventId)
				{
					// Special Adds Check event.

				case EVENT_CHECK_ADD_CC_DEATH:
					// Normal mode.
					if (!me->GetMap()->IsHeroic())
					{
						// Check for CC'ed adds, no groups died.
						if (GetLivingAddCount() == 9)
						{
							me->RemoveAurasDueToSpell(SPELL_WATCHFUL_EYE_2);
							me->RemoveAurasDueToSpell(SPELL_WATCHFUL_EYE_3);

							if (!me->HasAura(SPELL_WATCHFUL_EYE_1))
								me->AddAura(SPELL_WATCHFUL_EYE_1, me);

							if (GetSpearImpaledAdds() > 4)
								RemoveImpaledAddsAuras(GetSpearImpaledAdds() - 4);
						}
						// Check for CC'ed adds. Also one of the groups died, so summon reinforcements if not summoned.
						else if (GetLivingAddCount() == 6)
						{
							Talk(SAY_WATCHFUL_EYE);

							me->RemoveAurasDueToSpell(SPELL_WATCHFUL_EYE_1);
							me->RemoveAurasDueToSpell(SPELL_WATCHFUL_EYE_3);

							if (!me->HasAura(SPELL_WATCHFUL_EYE_2))
								me->AddAura(SPELL_WATCHFUL_EYE_2, me);

							if (GetSpearImpaledAdds() > 2)
								RemoveImpaledAddsAuras(GetSpearImpaledAdds() - 2);

							if (!reinforcementsSummoned)
							{
								Talk(SAY_ADD_GROUP_DIES);
								//DoCast(me, SPELL_RECKLESNESS_N);

								events.ScheduleEvent(EVENT_SUMMON_REINFORCEMENTS, 50000);
								reinforcementsSummoned = true;
							}
						}
						// Check for CC'ed adds. Also two of the groups died, so summon reinforcements if not summoned.
						else if (GetLivingAddCount() == 3)
						{
							Talk(SAY_WATCHFUL_EYE);

							me->RemoveAurasDueToSpell(SPELL_WATCHFUL_EYE_1);
							me->RemoveAurasDueToSpell(SPELL_WATCHFUL_EYE_2);

							if (!me->HasAura(SPELL_WATCHFUL_EYE_3))
								me->AddAura(SPELL_WATCHFUL_EYE_3, me);

							if (GetSpearImpaledAdds() > 0)
								RemoveImpaledAddsAuras(GetSpearImpaledAdds());

							if (!reinforcementsSummoned)
							{
								Talk(SAY_ADD_GROUP_DIES);
								//DoCast(me, SPELL_RECKLESNESS_N);

								events.ScheduleEvent(EVENT_SUMMON_REINFORCEMENTS, 50000);
								reinforcementsSummoned = true;
							}
						}
						// All of the groups died at once somehow (Weird :)), so summon reinforcements if not summoned.
						else if (GetLivingAddCount() == 0) // Shouldn't happen unless you kill all adds in 1 sec, but just to make sure.
						{
							if (!reinforcementsSummoned)
							{
								Talk(SAY_ADD_GROUP_DIES);
								//DoCast(me, SPELL_RECKLESNESS_N);

								events.ScheduleEvent(EVENT_SUMMON_REINFORCEMENTS, 50000);
								reinforcementsSummoned = true;
							}
						}
					}
					else // Heroic mode.
					{
						if (GetSpearImpaledAdds() > 3)
							RemoveImpaledAddsAuras(GetSpearImpaledAdds() - 3);

						if (GetLivingAddCount() < 9)
						{
							if (!reinforcementsSummoned)
							{
								Talk(SAY_ADD_GROUP_DIES);
								//DoCast(me, SPELL_RECKLESNESS_H);

								events.ScheduleEvent(EVENT_SUMMON_REINFORCEMENTS, 50000);
								reinforcementsSummoned = true;
							}
						}
					}
					events.ScheduleEvent(EVENT_CHECK_ADD_CC_DEATH, 100);
					break;

				case EVENT_SUMMON_REINFORCEMENTS:
					SummonReinforcements();
					break;

					// Elite Battlemasters event:
				case EVENT_KORTHIK_STRIKE:
					if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 150.0f, true))
						EliteBattleMastersCastStrike(target);
					events.ScheduleEvent(EVENT_KORTHIK_STRIKE, urand(34000, 50000));
					break;

					// Normal events.

				case EVENT_WHIRLING_BLADE:
					Talk(SAY_WHIRLING_BLADES);
					Talk(ANN_WHIRLING_BLADES);
					if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 25.0f, true))
						DoCast(target, SPELL_WHIRLING_BLADE);
					events.ScheduleEvent(EVENT_WHIRLING_BLADE, urand(48000, 64000));
					break;

				case EVENT_RAIN_OF_BLADES:
					Talk(SAY_RAIN_OF_BLADES);
					Talk(ANN_RAIN_OF_BLADES);
					DoCast(me, SPELL_RAIN_OF_BLADES);
					events.ScheduleEvent(EVENT_RAIN_OF_BLADES, urand(48000, 64000));
					break;

				case EVENT_WIND_BOMB:
					if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 150.0f, true))
						DoCast(target, SPELL_WIND_BOMB);
					events.ScheduleEvent(EVENT_WIND_BOMB, urand(18000, 24000));
					break;

				case EVENT_BERSERK_MELJARAK:
					DoCast(me, SPELL_BERSERK_MELJARAK);
					break;

				default: break;
				}
			}

			DoMeleeAttackIfReady();
		}

		/*** Special boss functions and calculations for adds. ***/

	private:

		void SummonSwarmAdds()
		{
			Position pos;

			for (uint8 i = 0; i < 3; i++)
			{
				me->GetRandomNearPosition(pos, 15.f);
				me->SummonCreature(NPC_KORTHIK_ELITE_BLADEMASTER, pos, TEMPSUMMON_MANUAL_DESPAWN);
			}

			for (uint8 i = 3; i < 6; i++)
			{
				me->GetRandomNearPosition(pos, 15.f);
				me->SummonCreature(NPC_SRATHIK_AMBER_TRAPPER, pos, TEMPSUMMON_MANUAL_DESPAWN);
			}

			for (uint8 i = 6; i < 9; i++)
			{
				me->GetRandomNearPosition(pos, 15.f);
				me->SummonCreature(NPC_ZARTHIK_BATTLE_MENDER, pos, TEMPSUMMON_MANUAL_DESPAWN);
			}
		}

		uint32 GetLivingAddCount()
		{
			uint32 livingAdds = 0;

			std::list<Creature*> allAdds;

			GetCreatureListWithEntryInGrid(allAdds, me, NPC_KORTHIK_ELITE_BLADEMASTER, 150.0f);
			GetCreatureListWithEntryInGrid(allAdds, me, NPC_SRATHIK_AMBER_TRAPPER, 150.0f);
			GetCreatureListWithEntryInGrid(allAdds, me, NPC_ZARTHIK_BATTLE_MENDER, 150.0f);

			if (allAdds.empty())
				return livingAdds;

			for (std::list<Creature*>::iterator iter = allAdds.begin(); iter != allAdds.end(); iter++)
				if ((*iter)->isAlive())
					livingAdds++;

			return livingAdds;
		}

		uint32 GetSpearImpaledAdds()
		{
			uint32 impaledAdds = 0;

			std::list<Creature*> allAdds;

			GetCreatureListWithEntryInGrid(allAdds, me, NPC_KORTHIK_ELITE_BLADEMASTER, 150.0f);
			GetCreatureListWithEntryInGrid(allAdds, me, NPC_SRATHIK_AMBER_TRAPPER, 150.0f);
			GetCreatureListWithEntryInGrid(allAdds, me, NPC_ZARTHIK_BATTLE_MENDER, 150.0f);

			if (allAdds.empty())
				return impaledAdds;

			for (std::list<Creature*>::iterator iter = allAdds.begin(); iter != allAdds.end(); iter++)
				if ((*iter)->HasCrowdControlAura())
					impaledAdds++;

			return impaledAdds;
		}

		void RemoveImpaledAddsAuras(uint32 numberToRemove)
		{
			uint32 addsToRemoveAuraFrom = numberToRemove;

			std::list<Creature*> allAdds;

			GetCreatureListWithEntryInGrid(allAdds, me, NPC_KORTHIK_ELITE_BLADEMASTER, 150.0f);
			GetCreatureListWithEntryInGrid(allAdds, me, NPC_SRATHIK_AMBER_TRAPPER, 150.0f);
			GetCreatureListWithEntryInGrid(allAdds, me, NPC_ZARTHIK_BATTLE_MENDER, 150.0f);

			if (allAdds.empty())
				return;

			for (std::list<Creature*>::iterator iter = allAdds.begin(); iter != allAdds.end(); iter++)
			{
				if ((*iter)->HasCrowdControlAura() && addsToRemoveAuraFrom > 0)
				{
					(*iter)->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_TAKE_DAMAGE);
					addsToRemoveAuraFrom--;
				}
			}
		}

		bool EliteBattleMastersDied()
		{
			std::list<Creature*> battleMasters;

			GetCreatureListWithEntryInGrid(battleMasters, me, NPC_KORTHIK_ELITE_BLADEMASTER, 150.0f);

			if (battleMasters.empty())
				return true;

			// Only need to check one, as their health is shared.
			std::list<Creature*>::iterator iter = battleMasters.begin();
			if ((*iter)->isAlive())
				return true;

			return false; // default.
		}

		bool AmberTrappersDied()
		{
			std::list<Creature*> amberTrappers;

			GetCreatureListWithEntryInGrid(amberTrappers, me, NPC_SRATHIK_AMBER_TRAPPER, 150.0f);

			if (amberTrappers.empty())
				return true;

			// Only need to check one, as their health is shared.
			std::list<Creature*>::iterator iter = amberTrappers.begin();
			if ((*iter)->isAlive())
				return true;

			return false; // default.
		}

		bool BattleMendersDied()
		{
			std::list<Creature*> battleMenders;

			GetCreatureListWithEntryInGrid(battleMenders, me, NPC_ZARTHIK_BATTLE_MENDER, 150.0f);

			if (battleMenders.empty())
				return true;

			// Only need to check one, as their health is shared.
			std::list<Creature*>::iterator iter = battleMenders.begin();
			if ((*iter)->isAlive())
				return true;

			return false; // default.
		}

		void SummonReinforcements()
		{
			Talk(SAY_SUMMON_REINFORCE);
			Talk(ANN_REINFORCEMENTS);

			// Safety check first :).
			if (GetLivingAddCount() == 9)
				return;
			// One group died.
			else if (GetLivingAddCount() == 6)
			{
				for (uint8 i = 0; i < 3; i++)
				{
					Position pos;
					me->GetRandomNearPosition(pos, 15.f);
					if (EliteBattleMastersDied()) // Summon Elite Battlemasters.
						me->SummonCreature(NPC_KORTHIK_ELITE_BLADEMASTER, pos, TEMPSUMMON_MANUAL_DESPAWN);
					else if (AmberTrappersDied()) // Summon Amber Trappers.
						me->SummonCreature(NPC_SRATHIK_AMBER_TRAPPER, pos, TEMPSUMMON_MANUAL_DESPAWN);
					else if (BattleMendersDied()) // Summon Battle Menders.
						me->SummonCreature(NPC_ZARTHIK_BATTLE_MENDER, pos, TEMPSUMMON_MANUAL_DESPAWN);
				}
			}
			// Two groups died.
			else if (GetLivingAddCount() == 3)
			{
				for (uint8 i = 0; i < 3; i++)
				{
					Position pos;
					if (EliteBattleMastersDied() && AmberTrappersDied()) // Summon Elite Battlemasters and Amber Trappers.
					{
						me->GetRandomNearPosition(pos, 15.f);
						me->SummonCreature(NPC_KORTHIK_ELITE_BLADEMASTER, pos, TEMPSUMMON_MANUAL_DESPAWN);
						me->GetRandomNearPosition(pos, 15.f);
						me->SummonCreature(NPC_SRATHIK_AMBER_TRAPPER, pos, TEMPSUMMON_MANUAL_DESPAWN);
					}
					else if (BattleMendersDied() && AmberTrappersDied()) // Summon Battle Menders and Amber Trappers.
					{
						me->GetRandomNearPosition(pos, 15.f);
						me->SummonCreature(NPC_ZARTHIK_BATTLE_MENDER, pos, TEMPSUMMON_MANUAL_DESPAWN);
						me->GetRandomNearPosition(pos, 15.f);
						me->SummonCreature(NPC_SRATHIK_AMBER_TRAPPER, pos, TEMPSUMMON_MANUAL_DESPAWN);
					}
					else if (EliteBattleMastersDied() && BattleMendersDied()) // Summon Elite Battlemasters and Battle Menders.
					{
						me->GetRandomNearPosition(pos, 15.f);
						me->SummonCreature(NPC_KORTHIK_ELITE_BLADEMASTER, pos, TEMPSUMMON_MANUAL_DESPAWN);
						me->GetRandomNearPosition(pos, 15.f);
						me->SummonCreature(NPC_ZARTHIK_BATTLE_MENDER, pos, TEMPSUMMON_MANUAL_DESPAWN);
					}
				}
			}
			// All groups died (WTF...Nuke blast!).
			else if (GetLivingAddCount() == 0)
			{
				for (uint8 i = 0; i < 3; i++) // Summon them all! :))
				{
					Position pos;
					me->GetRandomNearPosition(pos, 15.f);
					me->SummonCreature(NPC_KORTHIK_ELITE_BLADEMASTER, pos, TEMPSUMMON_MANUAL_DESPAWN);
					me->GetRandomNearPosition(pos, 15.f);
					me->SummonCreature(NPC_SRATHIK_AMBER_TRAPPER, pos, TEMPSUMMON_MANUAL_DESPAWN);
					me->GetRandomNearPosition(pos, 15.f);
					me->SummonCreature(NPC_ZARTHIK_BATTLE_MENDER, pos, TEMPSUMMON_MANUAL_DESPAWN);
				}
			}

			reinforcementsSummoned = false;
		}

		void EliteBattleMastersCastStrike(Unit* target)
		{
			std::list<Creature*> battleMasters;

			GetCreatureListWithEntryInGrid(battleMasters, me, NPC_KORTHIK_ELITE_BLADEMASTER, 150.0f);

			if (battleMasters.empty())
				return;

			// Cast it.
			for (std::list<Creature*>::iterator iter = battleMasters.begin(); iter != battleMasters.end(); ++iter)
				if ((*iter)->isAlive())
					(*iter)->CastSpell(target, SPELL_KORTHIK_STRIKE, false);
		}
	};

	CreatureAI* GetAI(Creature* creature) const
	{
		return new boss_wind_lord_meljarakAI(creature);
	}
};

// Wind Bomb: 67053.
class npc_wind_bomb_meljarak : public CreatureScript
{
public:
	npc_wind_bomb_meljarak() : CreatureScript("npc_wind_bomb_meljarak") { }

	struct npc_wind_bomb_meljarakAI : public ScriptedAI
	{
		npc_wind_bomb_meljarakAI(Creature* creature) : ScriptedAI(creature) { }

		EventMap events;

		void IsSummonedBy(Unit* summoner)
		{
			me->SetInCombatWithZone();
			me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
			me->SetReactState(REACT_PASSIVE);

			DoCast(me, SPELL_WIND_BOMB_THR_DMG);

			events.ScheduleEvent(EVENT_ARM, 3000);
			events.ScheduleEvent(EVENT_CHECK_PLAYER, 4000);
		}

		void UpdateAI(uint32 const diff)
		{
			events.Update(diff);

			while (uint32 eventId = events.ExecuteEvent())
			{
				switch (eventId)
				{
				case EVENT_ARM:
					DoCast(me, SPELL_WIND_BOMB_ARM);
					break;

				case EVENT_CHECK_PLAYER:
					if (Player* player = me->FindNearestPlayer(6.0f, true))
					{
						if (player->IsWithinDistInMap(me, 6.0f))
							DoCast(me, SPELL_WIND_BOMB_EXPLODE);
						else
							events.ScheduleEvent(EVENT_CHECK_PLAYER, 500);
					}
					else
						events.ScheduleEvent(EVENT_CHECK_PLAYER, 500);
					break;

				default: break;
				}
			}
		}
	};

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_wind_bomb_meljarakAI(creature);
	}
};

// Korthik Elite Blademaster: 62402. - Handled in boss script.

// Srathik Amber Trapper: 62405.

struct NonAlreadyAmberPrisoner : public std::unary_function<Unit*, bool>
{
public:
	NonAlreadyAmberPrisoner() { }
	bool operator()(Unit const* target) const
	{
		if (target->HasAura(SPELL_AMBER_PRISON_AURA))
			return false;

		return true;
	}
};


class npc_srathik_amber_trapper : public CreatureScript
{
public:
	npc_srathik_amber_trapper() : CreatureScript("npc_srathik_amber_trapper") { }

	struct npc_srathik_amber_trapperAI : public ScriptedAI
	{
		npc_srathik_amber_trapperAI(Creature* creature) : ScriptedAI(creature)
		{
			instance = creature->GetInstanceScript();
			charged = false;
		}

		InstanceScript* instance;
		EventMap events;
		bool charged;

		void Reset()
		{
			events.Reset();
		}

		void EnterCombat(Unit* /*who*/)
		{
			events.ScheduleEvent(EVENT_AMBER_PRISON, urand(13000, 47000));
			events.ScheduleEvent(EVENT_CORROSIVE_RESIN, urand(8000, 40000));
		}

		void UpdateAI(uint32 const diff)
		{
			if (!UpdateVictim() || me->HasUnitState(UNIT_STATE_CASTING))
				return;

			events.Update(diff);

			while (uint32 eventId = events.ExecuteEvent())
			{
				switch (eventId)
				{
				case EVENT_AMBER_PRISON:
					if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, NonAlreadyAmberPrisoner()))
						DoCast(target, SPELL_AMBER_PRISON);
					events.ScheduleEvent(EVENT_AMBER_PRISON, urand(35000, 70000));
					break;

				case EVENT_CORROSIVE_RESIN:
					if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 150.0f, true))
						DoCast(target, SPELL_CORROSIVE_RESIN);
					events.ScheduleEvent(EVENT_CORROSIVE_RESIN, urand(25000, 32000));
					break;

				default: break;
				}
			}

			DoMeleeAttackIfReady();
		}
	};

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_srathik_amber_trapperAI(creature);
	}
};

// Corrosive Resin Pool: 67046.
class npc_corrosive_resin_pool : public CreatureScript
{
public:
	npc_corrosive_resin_pool() : CreatureScript("npc_corrosive_resin_pool") { }

	struct npc_corrosive_resin_poolAI : public ScriptedAI
	{
		npc_corrosive_resin_poolAI(Creature* creature) : ScriptedAI(creature) { }

		void IsSummonedBy(Unit* /*summoner*/)
		{
			me->SetInCombatWithZone();
			me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
			me->SetReactState(REACT_PASSIVE);

			me->AddAura(SPELL_CORR_RESIN_POOL_A, me); // Damage aura.
		}
	};

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_corrosive_resin_poolAI(creature);
	}
};

// Amber Prison: 62531.
class npc_amber_prison_meljarak : public CreatureScript
{
public:
	npc_amber_prison_meljarak() : CreatureScript("npc_amber_prison_meljarak") { }

	struct npc_amber_prison_meljarakAI : public ScriptedAI
	{
		npc_amber_prison_meljarakAI(Creature* creature) : ScriptedAI(creature) { }

		Unit* target;
		uint32 timerChecktarget;

		void IsSummonedBy(Unit* summoner)
		{
			target = summoner;
			timerChecktarget = 500;
		}

		void DamageTaken(Unit* attacker, uint32& damage)
		{
			if (attacker->HasAura(SPELL_RESIDUE))
				damage = 0;
		}

		void UpdateAI(uint32 const diff)
		{
			if (timerChecktarget <= diff)
			{
				// Check if the debuff has expired.
				if (target && !target->HasAura(SPELL_AMBER_PRISON_AURA))
					me->DespawnOrUnsummon(100);

				timerChecktarget = 500;
			}
			else timerChecktarget -= diff;
		}

		void JustDied(Unit* killer)
		{
			if (target) target->RemoveAurasDueToSpell(SPELL_AMBER_PRISON_AURA);
			me->CastSpell(killer, SPELL_RESIDUE, false);
		}
	};

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_amber_prison_meljarakAI(creature);
	}
};

// Zarthik Battle Mender: 62408.
class npc_zarthik_battle_mender : public CreatureScript
{
public:
	npc_zarthik_battle_mender() : CreatureScript("npc_zarthik_battle_mender") { }

	struct npc_zarthik_battle_menderAI : public ScriptedAI
	{
		npc_zarthik_battle_menderAI(Creature* creature) : ScriptedAI(creature)
		{
			instance = creature->GetInstanceScript();
			charged = false;
		}

		InstanceScript* instance;
		EventMap events;
		bool charged;

		void Reset()
		{
			events.Reset();
		}

		void EnterCombat(Unit* /*who*/)
		{
			events.ScheduleEvent(EVENT_MENDING, urand(30000, 49000));
			events.ScheduleEvent(EVENT_QUICKENING, urand(12000, 28000));
		}

		void UpdateAI(uint32 const diff)
		{
			if (!UpdateVictim() || me->HasUnitState(UNIT_STATE_CASTING))
				return;

			events.Update(diff);

			while (uint32 eventId = events.ExecuteEvent())
			{
				switch (eventId)
				{
				case EVENT_MENDING:
				{
					uint32 result = urand(1, 3);
					switch (result)
					{
					case 1:
						if (Unit* ally = me->FindNearestCreature(NPC_KORTHIK_ELITE_BLADEMASTER, 150.0f, true))
							DoCast(ally, SPELL_MENDING);
						break;

					case 2:
						if (Unit* ally = me->FindNearestCreature(NPC_SRATHIK_AMBER_TRAPPER, 150.0f, true))
							DoCast(ally, SPELL_MENDING);
						break;

					case 3:
						DoCast(me, SPELL_MENDING);
						break;

					default: break;
					}
					events.ScheduleEvent(EVENT_MENDING, urand(37000, 62000));
					break;
				}
				case EVENT_QUICKENING:
				{
					DoCast(me, SPELL_QUICKENING);
					events.ScheduleEvent(EVENT_QUICKENING, urand(33000, 54000));
					break;
				}

				default: break;
				}
			}

			DoMeleeAttackIfReady();
		}
	};

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_zarthik_battle_menderAI(creature);
	}
};

// Corrosive Resin: 122064.
class spell_meljarak_corrosive_resin : public SpellScriptLoader
{
public:
	spell_meljarak_corrosive_resin() : SpellScriptLoader("spell_meljarak_corrosive_resin") { }

	class spell_meljarak_corrosive_resin_AuraScript : public AuraScript
	{
		PrepareAuraScript(spell_meljarak_corrosive_resin_AuraScript);

		void PeriodicTick(constAuraEffectPtr /*aurEff*/)
		{
			Unit* owner = GetOwner()->ToUnit();
			if (!owner)
				return;

			// Check target moving.
			if (owner->isMoving())
			{
				// Check the aura.
				if (AuraPtr resinAura = owner->GetAura(SPELL_CORROSIVE_RESIN))
				{
					// Remove the stacks.
					if (resinAura->GetStackAmount() > 1)
						resinAura->SetStackAmount(resinAura->GetStackAmount() - 1);
					else
						owner->RemoveAurasDueToSpell(SPELL_CORROSIVE_RESIN);

					// Summon the pool.
					owner->CastSpell(owner, SPELL_CORR_RESIN_POOL_S, false);
				}
			}
		}

		void Register()
		{
			OnEffectPeriodic += AuraEffectPeriodicFn(spell_meljarak_corrosive_resin_AuraScript::PeriodicTick, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
		}
	};

	AuraScript* GetAuraScript() const
	{
		return new spell_meljarak_corrosive_resin_AuraScript();
	}
};

// mending - 122147
class spell_mending : public SpellScriptLoader
{
public:
	spell_mending() : SpellScriptLoader("spell_mending") { }

	class spell_mending_SpellScript : public SpellScript
	{
		PrepareSpellScript(spell_mending_SpellScript);

		bool Load()
		{
			return true;
		}

		void CheckTargets(std::list<WorldObject*>& targets)
		{
			targets.clear();

			std::list<Creature*> tmpTargets;

			GetCaster()->GetCreatureListWithEntryInGridAppend(tmpTargets, NPC_KORTHIK_ELITE_BLADEMASTER, 150.0f);
			GetCaster()->GetCreatureListWithEntryInGridAppend(tmpTargets, NPC_SRATHIK_AMBER_TRAPPER, 150.0f);
			GetCaster()->GetCreatureListWithEntryInGridAppend(tmpTargets, NPC_ZARTHIK_BATTLE_MENDER, 150.0f);

			if (tmpTargets.empty())
				return;

			Creature* tmp = *tmpTargets.begin();
			float percentLife = 100.0f;

			for (auto itr : tmpTargets)
			{
				if (!itr->isAlive())
					continue;

				if (itr->GetHealthPct() < percentLife)
				{
					tmp = itr;
					percentLife = itr->GetHealthPct();
				}
			}

			targets.push_back(tmp);
		}

		void Register()
		{
			OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_mending_SpellScript::CheckTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
		}

		uint32 _targetCount;
	};

	SpellScript* GetSpellScript() const
	{
		return new spell_mending_SpellScript();
	}
};

void AddSC_boss_meljarak()
{
	new boss_wind_lord_meljarak();
	new npc_wind_bomb_meljarak();
	new npc_srathik_amber_trapper();
	new npc_corrosive_resin_pool();
	new npc_amber_prison_meljarak();
	new npc_zarthik_battle_mender();
	new spell_meljarak_corrosive_resin();
	new spell_mending();
}
