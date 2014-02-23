#include "CombatSimulation.h"

CombatSimulation::CombatSimulation()
	: hasLogged(false)
{
	
}

// sets the starting states based on the combat units within a radius of a given position
// this center will most likely be the position of the forwardmost combat unit we control
void CombatSimulation::setCombatUnits(const BWAPI::Position & center, const int radius)
{
	SparCraft::GameState s;

	BWAPI::Broodwar->drawCircleMap(center.x(), center.y(), 10, BWAPI::Colors::Red, true);

	std::vector<BWAPI::Unit *> ourCombatUnits;
	std::vector<UnitInfo> enemyCombatUnits;

	MapGrid::Instance().GetUnits(ourCombatUnits,   center, Options::Micro::COMBAT_REGROUP_RADIUS, true, false);
	InformationManager::Instance().getNearbyForce(enemyCombatUnits, center, BWAPI::Broodwar->enemy(), Options::Micro::COMBAT_REGROUP_RADIUS);

	BOOST_FOREACH (BWAPI::Unit * unit, ourCombatUnits)
	{
        if (InformationManager::Instance().isCombatUnit(unit->getType()) && SparCraft::System::isSupportedUnitType(unit->getType()))
		{
            try
            {
			    s.addUnit(getSparCraftUnit(unit));
            }
            catch (int e)
            {
                BWAPI::Broodwar->printf("Problem Adding Self Unit with ID: %d", unit->getID());
            }
		}
	}

	BOOST_FOREACH (UnitInfo ui, enemyCombatUnits)
	{
        if (!ui.type.isFlyer() && SparCraft::System::isSupportedUnitType(ui.type) && ui.completed)
		{
            try
            {
			    s.addUnit(getSparCraftUnit(ui));
            }
            catch (int e)
            {
                BWAPI::Broodwar->printf("Problem Adding Enemy Unit with ID: %d", ui.unitID);
            }
		}
	}

	s.finishedMoving();

	state = s;
}

// Gets a SparCraft unit from a BWAPI::Unit, used for our own units since we have all their info
const SparCraft::Unit CombatSimulation::getSparCraftUnit(BWAPI::Unit * unit) const
{
    return SparCraft::Unit( unit->getType(),
                            SparCraft::Position(unit->getPosition()), 
                            unit->getID(), 
                            getSparCraftPlayerID(unit->getPlayer()), 
                            unit->getHitPoints() + unit->getShields(), 
                            0,
		                    BWAPI::Broodwar->getFrameCount(), 
                            BWAPI::Broodwar->getFrameCount());	
}

// Gets a SparCraft unit from a UnitInfo struct, needed to get units of enemy behind FoW
const SparCraft::Unit CombatSimulation::getSparCraftUnit(const UnitInfo & ui) const
{
	BWAPI::UnitType type = ui.type;

    // this is a hack, treat medics as a marine for now
	if (type == BWAPI::UnitTypes::Terran_Medic)
	{
		type = BWAPI::UnitTypes::Terran_Marine;
	}

    return SparCraft::Unit( ui.type, 
                            SparCraft::Position(ui.lastPosition), 
                            ui.unitID, 
                            getSparCraftPlayerID(ui.player), 
                            ui.lastHealth, 
                            0,
		                    BWAPI::Broodwar->getFrameCount(), 
                            BWAPI::Broodwar->getFrameCount());	
}

ScoreType CombatSimulation::simulateCombat()
{
    try
    {
	    SparCraft::GameState s1(state);

        SparCraft::PlayerPtr selfNOK(new SparCraft::Player_NOKDPS(getSparCraftPlayerID(BWAPI::Broodwar->self())));

	    SparCraft::PlayerPtr enemyNOK(new SparCraft::Player_NOKDPS(getSparCraftPlayerID(BWAPI::Broodwar->enemy())));

	    SparCraft::Game g (s1, selfNOK, enemyNOK, 2000);

	    g.play();
	
	    ScoreType eval =  g.getState().eval(SparCraft::Players::Player_One, SparCraft::EvaluationMethods::LTD2).val();

	    BWAPI::Broodwar->drawTextScreen(240, 280, "Combat Sim : %d", eval);
        
	    return eval;
    }
    catch (int e)
    {
        BWAPI::Broodwar->printf("SparCraft FatalError, simulateCombat() threw");

        return -1;
    }
}

const SparCraft::GameState & CombatSimulation::getSparCraftState() const
{
	return state;
}

const IDType CombatSimulation::getSparCraftPlayerID(BWAPI::Player * player) const
{
	if (player == BWAPI::Broodwar->self())
	{
		return SparCraft::Players::Player_One;
	}
	else if (player == BWAPI::Broodwar->enemy())
	{
		return SparCraft::Players::Player_Two;
	}

	return SparCraft::Players::Player_None;
}

void CombatSimulation::logState(const SparCraft::GameState & state)
{
	if (hasLogged)
	{
		return;
	}

	std::stringstream log;

	//log << "State: [EVAL=" << state.eval(0) << ", SUMSQRT=(" << state.getTotalSumDPS(0) << "," << state.getTotalSumDPS(1) << ")\n";

	for (size_t p(0); p<SparCraft::Constants::Num_Players; ++p)
	{
		log << "Player " << p << " units:\n";

		for (size_t u(0); u<state.numUnits(p); ++u)
		{
			const SparCraft::Unit & unit(state.getUnit(p, u));

			log << "Unit " << u << ": " << unit.name() << " [HP=" << unit.currentHP() << ", X=" << unit.x() << ", Y=" << unit.y() << "]\n";
		}
	}

	//Logger::Instance().log(log.str());

	hasLogged = true;
}