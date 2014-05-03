/*#include "Common.h"
#include "SparCraftManager.h"

SparCraftManager::SparCraftManager() 
    : TT(new SparCraft::TranspositionTable())
    , gameOver(false)

{
    Search::StarcraftData::init();

    initialTotalSqrt[0] = 0;
    initialTotalSqrt[1] = 0;
}


void SparCraftManager::onStart()
{
    SparCraft::GameState state(extractGameState());

    initialTotalSqrt[0] = state.getTotalSumDPS(0);
    initialTotalSqrt[1] = state.getTotalSumDPS(1);
}

void SparCraftManager::update()
{
    if (!gameOver)
    {
        // for each unit that we control, do some book keeping
        BOOST_FOREACH (BWAPI::Unit * unit, BWAPI::Broodwar->self()->getUnits())
        {
            BWAPI::Broodwar->drawTextMap(unit->getPosition().x()-20, unit->getPosition().y()-12, "%d", unit->getID());

            // if it is a combat unit
            if (isCombatUnit(unit))
            {
                if (Options::Debug::DRAW_UALBERTABOT_DEBUG && unit->getTarget())
                {
                    BWAPI::Broodwar->drawLineMap(unit->getPosition().x()-2, unit->getPosition().y()-2, 	unit->getTarget()->getPosition().x(), unit->getTarget()->getPosition().y(), BWAPI::Colors::White);
                }
            }
        }

        performSparCraft();
    }
}



const MoveTuple	SparCraftManager::getMoveTuple(SparCraft::GameState & state, const IDType & playerModel)
{
    const IDType playerID = getPlayerID(BWAPI::Broodwar->self());

    //SparCraft::GameState state = extractGameState();
    //state.addUnit(2, BWAPI::UnitTypes::Protoss_Dragoon, Search::Players::Player_One, SparCraft::Position(0,0));
    //state.addUnit(2, BWAPI::UnitTypes::Zerg_Zergling, Search::Players::Player_Two, SparCraft::Position(0,0));
    //state.finishedMoving();

    state.setTotalSumDPS(initialTotalSqrt[0], initialTotalSqrt[1]);

    SparCraft::SparCraftParameters params;
    params.setMaxPlayer(Search::Players::Player_One);
    params.setSearchMethod(Search::SearchMethods::IDAlphaBeta);
    params.setPlayerToMoveMethod(Search::PlayerToMove::Alternate);
    params.setEvalMethod(Search::EvaluationMethods::ModelSimulation);
    params.setPlayerModel(Search::Players::Player_Two, Search::PlayerModels::No_Overkill_DPS, true);
    params.setScriptMoveFirstMethod(Search::PlayerModels::No_Overkill_DPS);
    params.setMaxDepth(50);
    params.setTimeLimit(42);

    SparCraft::AlphaBeta ab(params, TT);

    //ab.doSearch(state);

    // get the player based on the player model

    boost::shared_ptr<SparCraft::Player> player(new SparCraft::Player_NOK_AttackDPS(playerID));

    SparCraft::MoveArray moves;
    state.generateMoves(moves, player->ID());

    MoveTuple scriptMove = player->getMoveTuple(state, moves);
    //MoveTuple searchMove = ab.getResults().bestMoveTuple;


    SparCraft::GameState s(state);
    s.setTotalSumDPS(initialTotalSqrt[0], initialTotalSqrt[1]);

    BWAPI::Broodwar->drawTextScreen(40, 100, "Value: %d (%d, %d)", ab.getResults().abValue, ab.getResults().maxDepthReached, ab.getResults().nodesExpanded);
    BWAPI::Broodwar->drawTextScreen(40, 120, "Value: %d", s.evalSumDPS(0));

    //BWAPI::Broodwar->drawTextScreen(200, 200, "Initial Sqrt: %f %f", initialTotalSqrt[0], initialTotalSqrt[1]);
    //BWAPI::Broodwar->drawTextScreen(200, 220, "Current Sqrt: %f %f", state.getTotalSumDPS(0), state.getTotalSumDPS(1));

    //BWAPI::Broodwar->printf("Best Move Tuple %d - %d - %d %d %d %d", (int)m, (int)nm, (int)ab.getResults().nodesExpanded, (int)ab.getResults().maxDepthReached, (int)ab.getResults().abValue, (int)state.evalSumDPS(0));

    // return the move tuple
    return scriptMove;
}

void SparCraftManager::getMoves(SparCraft::GameState & state, std::vector<SparCraft::Move> & moveVec)
{
    const IDType playerID = getPlayerID(BWAPI::Broodwar->self());
    SparCraft::MoveArray moves;
    state.generateMoves(moves, playerID);

    SparCraft::PlayerPtr gsPlayer(new SparCraft::Player_PortfolioGreedySearch	(playerID, Search::PlayerModels::No_Overkill_DPS, 1));

    gsPlayer->getMoves(state, moves, moveVec);
}

void SparCraftManager::performSparCraft()
{
    SparCraft::GameState currentState = extractGameState();

    int currentFrame = BWAPI::Broodwar->getFrameCount();

    BOOST_FOREACH(BWAPI::Unit * unit, BWAPI::Broodwar->enemy()->getUnits())
    {
        BWAPI::Broodwar->drawTextMap(unit->getPosition().x(), unit->getPosition().y(), "%d", unit->getGroundWeaponCooldown());
    }

    // draw our units
    for (size_t u(0); u<currentState.numUnits(Search::Players::Player_One); ++u)
    {
        SparCraft::Unit & unit = currentState.getUnit(Search::Players::Player_One, u);
        BWAPI::Broodwar->drawCircleMap(unit.x(), unit.y(), 5, BWAPI::Colors::Green);
        BWAPI::Broodwar->drawCircleMap(unit.x(), unit.y(), unit.range(), BWAPI::Colors::Grey);

        std::pair<int, int> cooldown = getUnitCooldown(BWAPI::Broodwar->getUnit(unit.ID()), unit);

        BWAPI::Unit * realUnit = getUnit(unit);

        BWAPI::Broodwar->drawTextMap(unit.x(), unit.y(), "%d (%d %d %d)", unit.ID(), cooldown.first-currentFrame, cooldown.second-currentFrame, BWAPI::Broodwar->getUnit(unit.ID())->getGroundWeaponCooldown());
        BWAPI::Broodwar->drawLineMap(unit.x(), unit.y(), realUnit->getPosition().x(), realUnit->getPosition().y(), BWAPI::Colors::Purple);
    }

    // TODO: Check why zealots aren't being given commands

    // draw their units
    for (size_t u(0); u<currentState.numUnits(Search::Players::Player_Two); ++u)
    {
        SparCraft::Unit & unit = currentState.getUnit(Search::Players::Player_Two, u);
        drawUnitHP(BWAPI::Broodwar->getUnit(unit.ID()));
        //BWAPI::Broodwar->drawCircleMap(unit.x(), unit.y(), 5, BWAPI::Colors::Red);
    }

    // draw our moves if we are the player to move
    const IDType whoCanMove = currentState.whoCanMove();
    if ((whoCanMove == Search::Players::Player_One) || (whoCanMove == Search::Players::Player_Both))
    {
        // get the best move tuple from the current state
        //MoveTuple bestTuple = getMoveTuple(currentState, Search::PlayerModels::AlphaBeta);
        std::vector<SparCraft::Move> moveVec;
        getMoves(currentState, moveVec);

        // extract all of the moves possible from the current state
        SparCraft::MoveArray moves;
        currentState.generateMoves(moves, Search::Players::Player_One);

        // draw the best move for each unit
        for (size_t u(0); u<moves.numUnits(); ++u)
        {
            // the move for the first unit to move
            SparCraft::Move move = moveVec[u];

            // the unit for which the move is to be performed
            SparCraft::Unit & unit = currentState.getUnit(Search::Players::Player_One, move.unit());
            BWAPI::Broodwar->drawCircleMap(unit.x(), unit.y(), 5, BWAPI::Colors::Red);

            // draw the move this unit should do
            drawUnitMove(currentState, unit, move);
            drawUnitCooldown(BWAPI::Broodwar->getUnit(unit.ID()));
            drawUnitHP(BWAPI::Broodwar->getUnit(unit.ID()));

            // do the move
            doUnitMove(currentState, unit, move);
        }
    }
}

void SparCraftManager::doUnitMove(SparCraft::GameState & currentState, SparCraft::Unit & unit, SparCraft::Move & move)
{
    IDType enemyPlayer = (unit.player() + 1) % 2;

    BWAPI::Unit * u = BWAPI::Broodwar->getUnit(unit.ID());

    if (move._moveType == SparCraft::MoveTypes::ATTACK)
    {
        BWAPI::Broodwar->drawTextMap(unit.x()+5, unit.y()+5, "A");

        SparCraft::Unit & enemyUnit(currentState.getUnit(enemyPlayer, move._moveIndex));

        UnitCommandManager::Instance().smartAttackUnit(u, BWAPI::Broodwar->getUnit(enemyUnit.ID()));
    }
    else if (move._moveType == SparCraft::MoveTypes::MOVE)
    {
        BWAPI::Broodwar->drawTextMap(unit.x()+5, unit.y()+5, "M");

        SparCraft::Position pos(Search::Constants::Move_Dir[move._moveIndex][0], Search::Constants::Move_Dir[move._moveIndex][1]);
        SparCraft::Position dest(unit.x() + (pos.x() * 4*Search::Constants::Move_Distance), unit.y() + (pos.y() * 4*Search::Constants::Move_Distance));

        UnitCommandManager::Instance().smartMove(u, BWAPI::Position(dest.x(), dest.y()));
    }
    else if (move._moveType == SparCraft::MoveTypes::RELOAD)
    {
        UnitCommandManager::Instance().smartReload(u);
    }
}

void SparCraftManager::drawAttackDebug()
{
    char * trueFix = "\x07";
    char * falseFix = "\x06";

    BOOST_FOREACH(BWAPI::Unit * unit, BWAPI::Broodwar->self()->getUnits())
    {
        int x = unit->getPosition().x();
        int y = unit->getPosition().y() + 9;

        BWAPI::Broodwar->drawTextMap(x, y, "%s isAttacking", unit->isAttacking() ? trueFix : falseFix);
        BWAPI::Broodwar->drawTextMap(x, y+10, "%s isAttackFrame", unit->isAttackFrame() ? trueFix : falseFix);
        BWAPI::Broodwar->drawTextMap(x, y+20, "%s isMoving", unit->isMoving() ? trueFix : falseFix);
    }
}

void SparCraftManager::drawUnitMove(SparCraft::GameState & currentState, SparCraft::Unit & unit, SparCraft::Move & move)
{
    IDType enemyPlayer = (unit.player() + 1) % 2;

    if (move._moveType == SparCraft::MoveTypes::ATTACK)
    {
        SparCraft::Unit & enemyUnit(currentState.getUnit(enemyPlayer,move._moveIndex));

        BWAPI::Broodwar->drawLineMap(unit.x(), unit.y(), enemyUnit.x(), enemyUnit.y(), BWAPI::Colors::Cyan);
    }
    else if (move._moveType == SparCraft::MoveTypes::MOVE)
    {
        SparCraft::Position pos(Search::Constants::Move_Dir[move._moveIndex][0], Search::Constants::Move_Dir[move._moveIndex][1]);
        SparCraft::Position dest(unit.x() + (pos.x() * 32), unit.y() + (pos.y() * 32));

        BWAPI::Broodwar->drawLineMap(unit.x(), unit.y(), dest.x(), dest.y(), BWAPI::Colors::Yellow);
    }
}

void SparCraftManager::drawUnitCooldown(BWAPI::Unit * unit)
{
    double totalCooldown = unit->getType().groundWeapon().damageCooldown();
    double remainingCooldown = unit->getGroundWeaponCooldown();
    double percCooldown = remainingCooldown / (totalCooldown+1);

    int w = 32;
    int h = 4;

    int cw = w - (int)(w * percCooldown);
    int ch = h;

    int x1 = unit->getPosition().x() - w/2;
    int y1 = unit->getPosition().y() - 16;

    BWAPI::Broodwar->drawBoxMap(x1, y1, x1 + w, y1 + h, BWAPI::Colors::Grey, true);
    BWAPI::Broodwar->drawBoxMap(x1, y1, x1 + cw, y1 + ch, BWAPI::Colors::Red, true);
}

void SparCraftManager::drawUnitHP(BWAPI::Unit * unit)
{
    double totalHP = unit->getType().maxHitPoints() + unit->getType().maxShields();
    double currentHP = unit->getHitPoints() + unit->getShields();
    double percHP = currentHP / (totalHP+1);

    int w = 32;
    int h = 4;

    int cw = (int)(w * percHP);
    int ch = h;

    int x1 = unit->getPosition().x() - w/2;
    int y1 = unit->getPosition().y() - 12;

    BWAPI::Broodwar->drawBoxMap(x1, y1, x1 + w, y1 + h, BWAPI::Colors::Grey, true);
    BWAPI::Broodwar->drawBoxMap(x1, y1, x1 + cw, y1 + ch, BWAPI::Colors::Green, true);
}

SparCraft::SparCraftParameters SparCraftManager::getSearchParameters() const
{
    SparCraft::SparCraftParameters params;
    params.setMaxPlayer(Search::Players::Player_One);
    params.setSearchMethod(Search::SearchMethods::IDAlphaBeta);
    params.setPlayerToMoveMethod(Search::PlayerToMove::Alternate);
    //params.setPlayerModel(Search::Players::Player_Two, Search::PlayerModels::AttackClosest, true);
    params.setEvalMethod(Search::EvaluationMethods::ModelSimulation);
    params.setMaxDepth(50);
    params.setTimeLimit(35);

    return params;
}

const IDType SparCraftManager::getPlayerID(BWAPI::Player * player) const
{
    return (player == BWAPI::Broodwar->self()) ? Search::Players::Player_One : Search::Players::Player_Two;
}

const bool SparCraftManager::isCombatUnit(BWAPI::Unit * unit) const
{
    assert(unit != NULL);

    // no workers or buildings allowed
    if (unit && unit->getType().isWorker() || unit->getType().isBuilding())
    {
        return false;
    }

    // check for various types of combat units
    if (unit->getType().canAttack() || 
        unit->getType() == BWAPI::UnitTypes::Terran_Medic ||
        unit->getType() == BWAPI::UnitTypes::Protoss_High_Templar ||
        unit->getType() == BWAPI::UnitTypes::Protoss_Observer)
    {
        return true;
    }

    return false;
}

SparCraft::Player * SparCraftManager::getSparCraftPlayer(const IDType & playerModel, const IDType & player) const
{
    SparCraft::Player * p = NULL;

    if (playerModel == Search::PlayerModels::AlphaBeta)
    {
        SparCraft::Player_AlphaBeta * abp = new SparCraft::Player_AlphaBeta(player);
        abp->setParameters(getSearchParameters());
        p = abp;
    }
    else if (playerModel == Search::PlayerModels::AttackClosest)
    {
        p = new SparCraft::Player_AttackClosest(player);
    }
    else if (playerModel == Search::PlayerModels::Kiter)
    {
        p = new SparCraft::Player_Kiter(player);
    }
    else 
    {
        p = new SparCraft::Player_NOK_AttackDPS(player);
    }

    return p;
}

BWAPI::Unit * SparCraftManager::getUnit(SparCraft::Unit & unit)
{
    BOOST_FOREACH (BWAPI::Unit * u, BWAPI::Broodwar->getAllUnits())
    {
        if (u->getID() == unit.ID())
        {
            return u;
        }
    }

    return NULL;
}

const std::pair<int, int> SparCraftManager::getUnitCooldown(BWAPI::Unit * unit, SparCraft::Unit & u) const
{
    int attackCooldown(0);
    int moveCooldown(0);

    BWAPI::UnitCommand lastCommand = unit->getLastCommand();
    int lastCommandFrame = unit->getLastCommandFrame();
    int currentFrame = BWAPI::Broodwar->getFrameCount();
    int framesSinceCommand = currentFrame - lastCommandFrame;

    if ((unit->getType() == BWAPI::UnitTypes::Protoss_Dragoon) && (unit->getLastCommand().getType() == BWAPI::UnitCommandTypes::Attack_Unit))
    {
        // dragoons are one of only 2 unit types whose attack can be canceled by the in-game targeter being called too early so
        // this hack makes up for that by taking it's stop-delay into account
        attackCooldown = BWAPI::Broodwar->getFrameCount() + std::max(0, unit->getGroundWeaponCooldown()-Search::StarcraftData::getAttackFrames(unit->getType()).first);
    }
    else
    {
        attackCooldown = BWAPI::Broodwar->getFrameCount() + std::max(0, unit->getGroundWeaponCooldown()-2);
    }

    // if the last attack was an attack command
    if (lastCommand.getType() == BWAPI::UnitCommandTypes::Attack_Unit)
    {
        moveCooldown = BWAPI::Broodwar->getFrameCount() + std::max(0, u.attackInitFrameTime() - framesSinceCommand);

        //BWAPI::Broodwar->drawTextScreen(100,100, "%d, %d", attackCooldown-currentFrame, moveCooldown-currentFrame);
    }
    // if the last command was a move command
    else if (lastCommand.getType() == BWAPI::UnitCommandTypes::Move)
    {
        moveCooldown = currentFrame;
    }

    if (moveCooldown - BWAPI::Broodwar->getFrameCount() < 4 || unit->isMoving())
    {
        moveCooldown = currentFrame;
    }

    moveCooldown = std::min(moveCooldown, attackCooldown);

    return std::pair<int, int>(attackCooldown, moveCooldown);
}


// get an abstract GameState object from the current BWAPI state
SparCraft::GameState SparCraftManager::extractGameState()
{
    // construct a state with the current time
    SparCraft::GameState state;
    state.setTime(BWAPI::Broodwar->getFrameCount());

    // add each of our fighting units
    BOOST_FOREACH (BWAPI::Unit * unit, BWAPI::Broodwar->self()->getUnits())
    {
        if (isCombatUnit(unit))
        {
            SparCraft::Unit u(unit, getPlayerID(unit->getPlayer()), BWAPI::Broodwar->getFrameCount());
            std::pair<int, int> cd = getUnitCooldown(unit, u);
            u.setCooldown(cd.first, cd.second);
            state.addUnitWithID(u);
        }
    }

    BOOST_FOREACH (BWAPI::Unit * unit, BWAPI::Broodwar->enemy()->getUnits())
    {
        if (isCombatUnit(unit))
        {
            SparCraft::Unit u(unit, getPlayerID(unit->getPlayer()), BWAPI::Broodwar->getFrameCount());
            u.setCooldown(BWAPI::Broodwar->getFrameCount(), BWAPI::Broodwar->getFrameCount() + unit->getGroundWeaponCooldown());
            state.addUnitWithID(u);
        }
    }

    if (state.numUnits(0) == 0 && state.numUnits(1) == 0)
    {
        gameOver = true;
        //Logger::Instance().log("0\n");
    }
    else if (state.numUnits(1) == 0)
    {
        gameOver = true;
        //Logger::Instance().log("1\n");
    }
    else if (state.numUnits(0) == 0)
    {
        gameOver = true;
        //Logger::Instance().log("-1\n");
    }

    if (BWAPI::Broodwar->getFrameCount() > 5000)
    {
        std::stringstream ss;
        state.setTotalSumDPS(initialTotalSqrt[0], initialTotalSqrt[0]);
        ss << state.evalSumDPS(0) << "\n";

        gameOver = true;
        //Logger::Instance().log(ss.str());
    }

    if (gameOver)
    {	
        BWAPI::Broodwar->restartGame();
    }

    state.finishedMoving();
    return state;
}
*/