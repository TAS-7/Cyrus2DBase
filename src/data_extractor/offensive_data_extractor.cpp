/*
    Copyright:
    Cyrus2D
    Modified by Aref Sayareh, Nader Zare, Omid Amini
*/

#include "offensive_data_extractor.h"
#include "../chain_action/cooperative_action.h"
#include "../sample_player.h"
#include "../chain_action/action_state_pair.h"
#include <rcsc/player/world_model.h>
#include <random>
#include <time.h>
#include <vector>
#include <rcsc/common/logger.h>

#define ODEDebug

#define cm ","
//#define ADD_ELEM(key, value) fout << (value) << cm
#define ADD_ELEM(key, value) features.push_back(value)

double invalid_data_ = -2.0;
bool OffensiveDataExtractor::active = false;

using namespace rcsc;


OffensiveDataExtractor::OffensiveDataExtractor() :
        last_update_cycle(-1) {
}

OffensiveDataExtractor::~OffensiveDataExtractor() {
}


OffensiveDataExtractor::Option::Option() {
    cycle = true; //
    ball_pos = true;
    offside_count = true;
    unum = BOTH;
    pos = BOTH;
    relativePos = BOTH;
    polarPos = BOTH;
    isKicker = TM;
    isGhost = TM;
    openAnglePass = TM;
    nearestOppDist = TM;
    polarGoalCenter = TM;
    openAngleGoal = TM;
    in_offside = TM;
    use_convertor = true;
}


void OffensiveDataExtractor::init_file(DEState &state) {
    #ifdef ODEDebug
    dlog.addText(Logger::BLOCK, "start init_file");
    #endif
    time_t rawtime;
    struct tm *timeinfo;
    char buffer[80];

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    std::string dir = "/data1/aref/2d/data_yushan_pass_pred/";
    strftime(buffer, sizeof(buffer), "%Y-%m-%d-%H-%M-%S", timeinfo);
    std::string str(buffer);
    std::string rand_name = std::to_string(SamplePlayer::player_port);
    str += "_" + std::to_string(state.wm().self().unum()) + "_" + state.wm().opponentTeamName() + "_E" + rand_name + ".csv";

    fout = std::ofstream((dir + str).c_str());
    std::string header = get_header();
    #ifdef ODEDebug
        dlog.addText(Logger::BLOCK, header.c_str());
    #endif
    fout << header << std::endl;
}


std::string OffensiveDataExtractor::get_header() {
    std::string header = "";
    // Cycle and BALL
    if (option.cycle)
        header += "cycle,";
    if (option.ball_pos){
        header += std::string("ball_pos_x,ball_pos_y,ball_pos_r,ball_pos_t,");
    }

    if (option.offside_count){
        header += "offside_count,";
    }

    for (int i = 1; i <= 11; i++) {
        if (option.unum == TM || option.unum == BOTH)
            header += "p_l_" + std::to_string(i) + "_unum,";
        if (option.pos == TM || option.pos == BOTH) {
            header += "p_l_" + std::to_string(i) + "_pos_x,";
            header += "p_l_" + std::to_string(i) + "_pos_y,";
        }
        if (option.polarPos == TM || option.polarPos == BOTH) {
            header += "p_l_" + std::to_string(i) + "_pos_r,";
            header += "p_l_" + std::to_string(i) + "_pos_t,";
        }
        if (option.relativePos == TM || option.relativePos == BOTH) {
            header += "p_l_" + std::to_string(i) + "_kicker_x,";
            header += "p_l_" + std::to_string(i) + "_kicker_y,";
            header += "p_l_" + std::to_string(i) + "_kicker_r,";
            header += "p_l_" + std::to_string(i) + "_kicker_t,";
        }
        if (option.in_offside == TM)
            header += "p_l_" + std::to_string(i) + "_in_offside,";
        if (option.isKicker == TM || option.isKicker == BOTH)
            header += "p_l_" + std::to_string(i) + "_is_kicker,";
        if (option.isGhost == TM || option.isGhost == BOTH)
            header += "p_l_" + std::to_string(i) + "_is_ghost,";
        if (option.openAnglePass == TM || option.openAnglePass == BOTH) {
            header += "p_l_" + std::to_string(i) + "_pass_dist,";
            header += "p_l_" + std::to_string(i) + "_pass_opp1_dist,";
            header += "p_l_" + std::to_string(i) + "_pass_opp1_dist_proj_to_opp,";
            header += "p_l_" + std::to_string(i) + "_pass_opp1_dist_proj_to_kicker,";
            header += "p_l_" + std::to_string(i) + "_pass_opp1_open_angle,";
            header += "p_l_" + std::to_string(i) + "_pass_opp1_dist_diffbody,";
        }
        if (option.nearestOppDist == TM || option.nearestOppDist == BOTH){
            header += "p_l_" + std::to_string(i) + "_near1_opp_dist,";
            header += "p_l_" + std::to_string(i) + "_near1_opp_angle,";
            header += "p_l_" + std::to_string(i) + "_near1_opp_diffbody,";
        }
        if (option.polarGoalCenter == TM || option.polarGoalCenter == BOTH) {
            header += "p_l_" + std::to_string(i) + "_angle_goal_center_r,";
            header += "p_l_" + std::to_string(i) + "_angle_goal_center_t,";
        }
        if (option.openAngleGoal == TM || option.openAngleGoal == BOTH)
            header += "p_l_" + std::to_string(i) + "_open_goal_angle,";
    }
    for (int i = 1; i <= 15; i++) {
        if (option.unum == OPP || option.unum == BOTH)
            header += "p_r_" + std::to_string(i) + "_unum,";
        if (option.pos == OPP || option.pos == BOTH) {
            header += "p_r_" + std::to_string(i) + "_pos_x,";
            header += "p_r_" + std::to_string(i) + "_pos_y,";
        }
        if (option.polarPos == OPP || option.polarPos == BOTH) {
            header += "p_r_" + std::to_string(i) + "_pos_r,";
            header += "p_r_" + std::to_string(i) + "_pos_t,";
        }
        if (option.relativePos == OPP || option.relativePos == BOTH) {
            header += "p_r_" + std::to_string(i) + "_kicker_x,";
            header += "p_r_" + std::to_string(i) + "_kicker_y,";
            header += "p_r_" + std::to_string(i) + "_kicker_r,";
            header += "p_r_" + std::to_string(i) + "_kicker_t,";
        }
        if (option.isKicker == OPP || option.isKicker == BOTH)
            header += "p_r_" + std::to_string(i) + "_is_kicker,";
        if (option.isGhost == OPP || option.isGhost == BOTH)
            header += "p_r_" + std::to_string(i) + "_is_ghost,";
        if (option.openAnglePass == OPP || option.openAnglePass == BOTH) {
            header += "p_r_" + std::to_string(i) + "_pass_angle,";
            header += "p_r_" + std::to_string(i) + "_pass_dist,";
        }
        if (option.nearestOppDist == OPP || option.nearestOppDist == BOTH){
            header += "p_r_" + std::to_string(i) + "_near1_opp_dist,";
            header += "p_r_" + std::to_string(i) + "_near1_opp_angle,";
            header += "p_r_" + std::to_string(i) + "_near2_opp_dist,";
            header += "p_r_" + std::to_string(i) + "_near2_opp_angle,";
        }
        if (option.polarGoalCenter == OPP || option.polarGoalCenter == BOTH) {
            header += "p_r_" + std::to_string(i) + "_angle_coal_center_r,";
            header += "p_r_" + std::to_string(i) + "_angle_goal_center_t,";
        }
        if (option.openAngleGoal == OPP || option.openAngleGoal == BOTH)
            header += "p_r_" + std::to_string(i) + "_open_goal_Angle,";
    }
    header += "out_category,out_target_x,out_target_y,out_unum,out_unum_index,out_ball_speed,out_ball_dir,out_desc,";
    return header;
}

void OffensiveDataExtractor::generate_save_data(const WorldModel & wm, const CooperativeAction &action,bool update_shoot) {
    if(!OffensiveDataExtractor::active)
        return;
    if (last_update_cycle == wm.time().cycle())
        return;
    if (!wm.self().isKickable())
        return;
    if (wm.gameMode().type() != rcsc::GameMode::PlayOn)
        return;

    #ifdef ODEDebug
    dlog.addText(Logger::BLOCK, "start update");
    #endif
    DEState state = DEState(wm);
    if (state.kicker() == nullptr)
        return;

    if (!fout.is_open()) {
        init_file(state);
    }
    last_update_cycle = wm.time().cycle();
    features.clear();

    if (!update_shoot){
        if (
                action.category() > 2
                ||
                !action.targetPoint().isValid()
                ||
                action.targetPlayerUnum() > 11
                ||
                action.targetPlayerUnum() < 1
                )
            return;
    }

    // cycle
    if (option.cycle)
        ADD_ELEM("cycle", convertor_cycle(last_update_cycle));

    // ball
    extract_ball(state);

    // kicker
    extract_kicker(state);

    // players
    extract_players(state);

    // output
    if (!update_shoot){
        extract_output(state,
                       action.category(),
                       action.targetPoint(),
                       action.targetPlayerUnum(),
                       action.description(),
                       action.firstBallSpeed());
    }
    for (int i = 0; i < features.size(); i++){
        if ( i == features.size() - 1){
            fout<<features[i];
        }else{
            fout<<features[i]<<",";
        }
    }
    fout<<std::endl;
}

std::vector<double> OffensiveDataExtractor::get_data(DEState & state){
    features.clear();
    if (option.cycle)
        ADD_ELEM("cycle", convertor_cycle(state.cycle()));

    // ball
    extract_ball(state);

    // kicker
    extract_kicker(state);

    // players
    extract_players(state);
    return features;
}

OffensiveDataExtractor &OffensiveDataExtractor::i() {
    static OffensiveDataExtractor instance;
    return instance;
}


void OffensiveDataExtractor::extract_ball(DEState &state) {
    #ifdef ODEDebug
    dlog.addText(Logger::BLOCK, "start extract_ball");
    #endif
    if (option.ball_pos){
        if (state.ball().posValid()) {
            ADD_ELEM("p_x", convertor_x(state.ball().pos().x));
            ADD_ELEM("p_y", convertor_y(state.ball().pos().y));
            ADD_ELEM("p_r", convertor_dist(state.ball().pos().r()));
            ADD_ELEM("p_t", convertor_angle(state.ball().pos().th().degree()));
            #ifdef ODEDebug
            dlog.addText(Logger::BLOCK, "##add ball pos x y r t");
            #endif
        } else {
            ADD_ELEM("p_x", invalid_data_);
            ADD_ELEM("p_y", invalid_data_);
            ADD_ELEM("p_r", invalid_data_);
            ADD_ELEM("p_t", invalid_data_);
            #ifdef ODEDebug
            dlog.addText(Logger::BLOCK, "##@add ball invalid pos x y r t");
            #endif
        }
    }
    if (option.offside_count){
        ADD_ELEM("offside_count", convertor_counts(state.offsideLineCount()));
        #ifdef ODEDebug
        dlog.addText(Logger::BLOCK, "##add offside count");
        #endif
    }
}

void OffensiveDataExtractor::extract_kicker(DEState &state) {
    extract_drible_angles(state);
}

void OffensiveDataExtractor::extract_players(DEState &state) {
    auto players = sort_players3(state);
    #ifdef ODEDebug
    dlog.addText(Logger::BLOCK, "start extract_players");
    #endif
    for (uint i = 0; i < players.size(); i++) {
        #ifdef ODEDebug
        dlog.addText(Logger::BLOCK, "------------------------------");
        dlog.addText(Logger::BLOCK, "player %d in players list", i);
        #endif
        DEPlayer *player = players[i];
        if (player == nullptr) {
            add_null_player(invalid_data_,
                            (i <= 10 ? TM : OPP));
            #ifdef ODEDebug
            dlog.addText(Logger::BLOCK, "## add invalid data");
            #endif
            continue;
        }
        #ifdef ODEDebug
            dlog.addText(Logger::BLOCK, "## start extracting for p side%d unum%d", player->side(), player->unum());
        #endif
        ODEDataSide side = player->side() == state.ourSide() ? TM : OPP;
        extract_base_data(player, side, state);
        extract_pos(player, state, side);

        if (option.isKicker == side || option.isKicker == BOTH) {
            if (player->unum() == state.kicker()->unum()) {
                ADD_ELEM("is_kicker", 1);
            } else
                ADD_ELEM("is_kicker", 0);
        }
        if (option.isGhost == side || option.isGhost == BOTH) {
            if (player->isGhost()) {
                ADD_ELEM("is_ghost", 1);
            } else
                ADD_ELEM("is_ghost", 0);
        }

        extract_pass_angle(player, state, side);
        extract_goal_polar(player, side);
        extract_goal_open_angle(player, state, side);
    }
}

std::vector<DEPlayer *> OffensiveDataExtractor::sort_players(DEState &state) {
    static int cycle = 0;
    static std::vector<DEPlayer *> tms;
    if (state.wm().time().cycle() == cycle){
        return tms;
    }

    cycle = state.wm().time().cycle();
    tms.clear();
    std::vector<DEPlayer *> opps;
    tms.clear();
    opps.clear();

    for (int i = 1; i <= 11; i++){
        DEPlayer * player = state.ourPlayer(i);
        if (player == nullptr || player->unum() < 0 || !player->pos().isValid() || player->isGhost()){
            tms.push_back(nullptr);
            continue;
        }
        tms.push_back(player);
    }

    for (int i = 1; i <= 11; i++){
        DEPlayer * player = state.theirPlayer(i);
        if (player == nullptr || player->unum() < 0 || !player->pos().isValid()){
            opps.push_back(nullptr);
            continue;
        }
        opps.push_back(player);
    }

    tms.insert(tms.end(), opps.begin(), opps.end());

    return tms;
}

void OffensiveDataExtractor::add_null_player(int unum, ODEDataSide side) {
    if (option.side == side || option.side == BOTH)
        ADD_ELEM("side", invalid_data_);
    if (option.unum == side || option.unum == BOTH)
        ADD_ELEM("unum", unum);
    if (option.pos == side || option.pos == BOTH) {
        ADD_ELEM("pos_x", invalid_data_);
        ADD_ELEM("pos_y", invalid_data_);
    }
    if (option.polarPos == side || option.polarPos == BOTH) {
        ADD_ELEM("pos_r", invalid_data_);
        ADD_ELEM("pos_t", invalid_data_);
    }
    if (option.relativePos == side || option.relativePos == BOTH) {
        ADD_ELEM("kicker_x", invalid_data_);
        ADD_ELEM("kicker_y", invalid_data_);
        ADD_ELEM("kicker_r", invalid_data_);
        ADD_ELEM("kicker_t", invalid_data_);
    }
    if (option.in_offside == side || option.in_offside == BOTH) {
        ADD_ELEM("in_offside", invalid_data_);
    }
    if (option.isKicker == side || option.isKicker == BOTH)
        ADD_ELEM("is_kicker", invalid_data_);
    if (option.isGhost == side || option.isGhost == BOTH) {
        ADD_ELEM("is_ghost", invalid_data_);
    }
    if (option.openAnglePass == side || option.openAnglePass == BOTH) {
        ADD_ELEM("pass_dist", invalid_data_);
        ADD_ELEM("pass_opp1_dist", invalid_data_);
        ADD_ELEM("pass_opp1_angle", invalid_data_);
        ADD_ELEM("pass_opp1_dist_line", invalid_data_);
        ADD_ELEM("pass_opp1_dist_proj", invalid_data_);
        ADD_ELEM("pass_opp1_dist_diffbody", invalid_data_);
    }
    if (option.nearestOppDist == side || option.nearestOppDist == BOTH){
        ADD_ELEM("opp1_dist", invalid_data_);
        ADD_ELEM("opp1_angle", invalid_data_);
        ADD_ELEM("opp1_diffbody", invalid_data_);
    }

    if (option.polarGoalCenter == side || option.polarGoalCenter == BOTH) {
        ADD_ELEM("angle_goal_center_r", invalid_data_);
        ADD_ELEM("angle_goal_center_t", invalid_data_);
    }
    if (option.openAngleGoal == side || option.openAngleGoal == BOTH)
        ADD_ELEM("open_goal_angle", invalid_data_);
}

void OffensiveDataExtractor::extract_output(DEState &state,
                                   int category,
                                   const rcsc::Vector2D &target,
                                   const int &unum,
                                   const char *desc,
                                   double ball_speed) {
    ADD_ELEM("category", category);
    ADD_ELEM("target_x", convertor_x(target.x));
    ADD_ELEM("target_y", convertor_y(target.y));
    ADD_ELEM("unum", unum);
    ADD_ELEM("unum_index", find_unum_index(state, unum));
    ADD_ELEM("ball_speed", convertor_bv(ball_speed));
    ADD_ELEM("ball_dir", convertor_angle((target - state.ball().pos()).th().degree()));
    if (category == 2) {
        if (std::string(desc) == "strictDirect") {
            ADD_ELEM("desc", 0);
        } else if (std::string(desc) == "strictLead") {
            ADD_ELEM("desc", 1);
        } else if (std::string(desc) == "strictThrough") {
            ADD_ELEM("desc", 2);
        } else if (std::string(desc) == "cross") {
            ADD_ELEM("desc", 3);
        }
    } else {
        ADD_ELEM("desc", 4);
    }
}

void OffensiveDataExtractor::extract_pass_angle(DEPlayer *player, DEState &state, ODEDataSide side) {
    Vector2D ball_pos = state.ball().pos();
    Vector2D tm_pos = player->pos();
    int max_pos_count = 30;
    if (player->unum() == state.kicker()->unum() && player->side() == state.ourSide()){
        max_pos_count = 20;
    }
    #ifdef ODEDebug
    dlog.addText(Logger::BLOCK, "##start extract pass angle");
    #endif
    if (!ball_pos.isValid() || !tm_pos.isValid() || !state.ball().posValid() || player->posCount() > max_pos_count){
        if (option.openAnglePass == side || option.openAnglePass == BOTH) {
            #ifdef ODEDebug
            dlog.addText(Logger::BLOCK, "#### add invalid data for open angle pass");
            #endif
            ADD_ELEM("pass_dist", invalid_data_);
            ADD_ELEM("pass_opp1_dist", invalid_data_);
            ADD_ELEM("pass_opp1_angle", invalid_data_);
            ADD_ELEM("pass_opp1_dist_line", invalid_data_);
            ADD_ELEM("pass_opp1_dist_proj", invalid_data_);
            ADD_ELEM("pass_opp1_dist_diffbody", invalid_data_);
        }
        if (option.nearestOppDist == side || option.nearestOppDist == BOTH){
            #ifdef ODEDebug
            dlog.addText(Logger::BLOCK, "#### add invalid for nearest opp dist");
            #endif
            ADD_ELEM("opp1_dist", invalid_data_);
            ADD_ELEM("opp1_angle", invalid_data_);
            ADD_ELEM("opp1_diffbody", invalid_data_);
        }
        return;
    }
    std::vector<std::pair<double, double>> opp_dist_angle;
    std::vector<std::pair<double, double>> opp_dist_body_diff;
    std::vector<ODEOpenAngle> candidates;
    for (const auto& opp: state.opponents()) {
        ODEOpenAngle candid;
        #ifdef ODEDebug
        dlog.addText(Logger::BLOCK, "######want to check opp %d", opp->unum());
        #endif
        if (!opp->pos().isValid()) continue;
        candid.dist_self_to_opp = opp->pos().dist(ball_pos);
        opp_dist_angle.push_back(std::make_pair(opp->pos().dist(ball_pos), (opp->pos() - ball_pos).th().degree()));
        if (opp->bodyValid())
            opp_dist_body_diff.push_back(std::make_pair(opp->pos().dist(ball_pos), ((ball_pos - opp->pos()).th() - opp->body()).abs()));
        else
            opp_dist_body_diff.push_back(std::make_pair(opp->pos().dist(ball_pos), invalid_data_));
        AngleDeg diff = (tm_pos - ball_pos).th() - (opp->pos() - ball_pos).th();
        #ifdef ODEDebug
        dlog.addText(Logger::BLOCK, "######check opp %d in %.1f,%.1f, diff:%.1f", opp->unum(), opp->pos().x, opp->pos().y, diff.degree());
        #endif
        if (diff.abs() > 60)
            continue;
        if (opp->pos().dist(ball_pos) > tm_pos.dist(ball_pos) + 10.0)
            continue;
        candid.open_angle = diff.abs();
        Vector2D proj_pos = Line2D(ball_pos, tm_pos).projection(opp->pos());
        candid.dist_opp_proj = proj_pos.dist(opp->pos());
        candid.dist_self_to_opp_proj = proj_pos.dist(state.kicker()->pos());
        if (opp->bodyValid())
            candid.opp_body_diff = ((proj_pos - opp->pos()).th() - opp->body()).abs();
        else
            candid.opp_body_diff = invalid_data_;
        candidates.push_back(candid);

    }
    if (option.openAnglePass == side || option.openAnglePass == BOTH) {
        auto open_angle_sorter = [](ODEOpenAngle &p1, ODEOpenAngle &p2) -> bool {
            return p1.open_angle < p2.open_angle;
        };
        std::sort(candidates.begin(), candidates.end(), open_angle_sorter);

        ADD_ELEM("pass_dist", convertor_dist(ball_pos.dist(tm_pos)));
        if (candidates.size() >= 1){
            #ifdef ODEDebug
            dlog.addText(Logger::BLOCK, "###### add opp angle pass first");
            #endif
            ADD_ELEM("pass_opp1_dist", convertor_dist(candidates[0].dist_self_to_opp));
            ADD_ELEM("pass_opp1_dist_proj_to_opp", convertor_dist(candidates[0].dist_opp_proj));
            ADD_ELEM("pass_opp1_dist_proj_to_kicker", convertor_dist(candidates[0].dist_self_to_opp_proj));
            ADD_ELEM("pass_opp1_open_angle", convertor_angle(candidates[0].open_angle));
            if (candidates[0].opp_body_diff != invalid_data_)
                ADD_ELEM("pass_opp1_dist_diffbody", convertor_angle(candidates[0].opp_body_diff));
            else
                ADD_ELEM("pass_opp1_dist_diffbody", invalid_data_);
        }
        else{
            #ifdef ODEDebug
            dlog.addText(Logger::BLOCK, "###### add opp angle pass first invalid");
            #endif
            ADD_ELEM("pass_opp1_dist", invalid_data_);
            ADD_ELEM("pass_opp1_angle", invalid_data_);
            ADD_ELEM("pass_opp1_dist_line", invalid_data_);
            ADD_ELEM("pass_opp1_dist_proj", invalid_data_);
            ADD_ELEM("pass_opp1_dist_diffbody", invalid_data_);
        }
    }
    if (option.nearestOppDist == side || option.nearestOppDist == BOTH){
        std::sort(opp_dist_angle.begin(), opp_dist_angle.end());
        std::sort(opp_dist_body_diff.begin(), opp_dist_body_diff.end());
        if (opp_dist_angle.size() >= 1){
            #ifdef ODEDebug
            dlog.addText(Logger::BLOCK, "###### add opp pass dist first");
            #endif
            ADD_ELEM("opp1_dist", convertor_dist(opp_dist_angle[0].first));
            ADD_ELEM("opp1_angle", convertor_angle(opp_dist_angle[0].second));
            if (opp_dist_body_diff[0].second != invalid_data_)
                ADD_ELEM("opp1_diffbody", convertor_angle(opp_dist_body_diff[0].second));
            else
                ADD_ELEM("opp1_diffbody", invalid_data_);
        }
        else{
            #ifdef ODEDebug
            dlog.addText(Logger::BLOCK, "###### add opp pass dist first invalid");
            #endif
            ADD_ELEM("opp1_dist", invalid_data_);
            ADD_ELEM("opp1_angle", invalid_data_);
            ADD_ELEM("opp1_diffbody", invalid_data_);
        }
    }
}

void OffensiveDataExtractor::extract_pos(DEPlayer *player, DEState &state, ODEDataSide side) {
    int max_pos_count = 30;
    if (player->unum() == state.kicker()->unum() && player->side() == state.ourSide())
        max_pos_count = 20;

    if (player->posCount() <= max_pos_count and player->pos().isValid()){
        if (option.pos == side || option.pos == BOTH) {
            ADD_ELEM("pos_x", convertor_x(player->pos().x));
            ADD_ELEM("pos_y", convertor_y(player->pos().y));
        }
        if (option.polarPos == side || option.polarPos == BOTH) {
            ADD_ELEM("pos_r", convertor_dist(player->pos().r()));
            ADD_ELEM("pos_t", convertor_angle(player->pos().th().degree()));
        }
        Vector2D rpos = player->pos() - state.kicker()->pos();
        if (option.relativePos == side || option.relativePos == BOTH) {
            ADD_ELEM("kicker_x", convertor_dist_x(rpos.x));
            ADD_ELEM("kicker_y", convertor_dist_y(rpos.y));
            ADD_ELEM("kicker_r", convertor_dist(rpos.r()));
            ADD_ELEM("kicker_t", convertor_angle(rpos.th().degree()));
        }
        if (option.in_offside == side || option.in_offside == BOTH) {
            if (player->pos().x > state.offsideLineX()) {
                ADD_ELEM("pos_offside", 1);
            } else {
                ADD_ELEM("pos_offside", 0);
            }
        }
    }else{
        if (option.pos == side || option.pos == BOTH) {
            ADD_ELEM("pos_x", invalid_data_);
            ADD_ELEM("pos_y", invalid_data_);
        }
        if (option.polarPos == side || option.polarPos == BOTH) {
            ADD_ELEM("pos_r", invalid_data_);
            ADD_ELEM("pos_t", invalid_data_);
        }
        if (option.relativePos == side || option.relativePos == BOTH) {
            ADD_ELEM("kicker_x", invalid_data_);
            ADD_ELEM("kicker_y", invalid_data_);
            ADD_ELEM("kicker_r", invalid_data_);
            ADD_ELEM("kicker_t", invalid_data_);
        }
        if (option.in_offside == side || option.in_offside == BOTH) {
            ADD_ELEM("pos_offside", invalid_data_);
        }
    }
}

void OffensiveDataExtractor::extract_goal_polar(DEPlayer *player, ODEDataSide side) {
    if (!(option.polarGoalCenter == side || option.polarGoalCenter == BOTH))
        return;
    if (!player->pos().isValid()){
        ADD_ELEM("angle_goal_center_r", invalid_data_);
        ADD_ELEM("angle_goal_center_t", invalid_data_);
        return;
    }
    Vector2D goal_center = Vector2D(52, 0);
    ADD_ELEM("angle_goal_center_r", convertor_dist((goal_center - player->pos()).r()));
    ADD_ELEM("angle_goal_center_t", convertor_angle((goal_center - player->pos()).th().degree()));
}

void OffensiveDataExtractor::extract_goal_open_angle(DEPlayer *player,
                                            DEState &state,
                                            ODEDataSide side) {
    if (!(option.openAngleGoal == side || option.openAngleGoal == BOTH))
        return;
    if (!player->pos().isValid()){
        ADD_ELEM("goal_open_angle", invalid_data_);
        return;
    }
    Vector2D goal_t = Vector2D(52, -7);
    Vector2D goal_b = Vector2D(52, 7);
    Triangle2D player_goal_area = Triangle2D(goal_b, goal_t, player->pos());

    std::vector<Vector2D> players_in_area;

    for (const auto& opp: state.theirPlayers()){
        if (!opp->pos().isValid())
            continue;
        if (!player_goal_area.contains(opp->pos()))
            continue;

        players_in_area.push_back(opp->pos());
    }
    players_in_area.push_back(goal_t);
    players_in_area.push_back(goal_b);

    std::sort(players_in_area.begin(), players_in_area.end(),
              [player](Vector2D p1, Vector2D p2) -> bool {
                  return (p1 - player->pos()).th().degree() < (p2 - player->pos()).th().degree();
              });

    double max_open_angle = 0;
    for (uint i = 1; i < players_in_area.size(); i++) {
        double angle_diff = fabs((players_in_area[i] - player->pos()).th().degree()
                                 - (players_in_area[i - 1] - player->pos()).th().degree());
        if (angle_diff > max_open_angle)
            max_open_angle = angle_diff;
    }
    ADD_ELEM("goal_open_angle", convertor_angle(max_open_angle));
}

void OffensiveDataExtractor::extract_base_data(DEPlayer *player, ODEDataSide side, DEState &state) {
    if (option.side == side || option.side == BOTH){
        #ifdef ODEDebug
        dlog.addText(Logger::BLOCK, "#### add side %d", player->side());
        #endif
        ADD_ELEM("side", player->side());
    }
    if (option.unum == side || option.unum == BOTH){
        if (player->unum() == -1){
            ADD_ELEM("unum", invalid_data_);
            #ifdef ODEDebug
            dlog.addText(Logger::BLOCK, "#### add invalid unum");
            #endif
        }else{
            ADD_ELEM("unum", convertor_unum(player->unum()));
            #ifdef ODEDebug
            dlog.addText(Logger::BLOCK, "#### add unum %d", player->unum());
            #endif
        }
    }
}

double OffensiveDataExtractor::convertor_x(double x) {
    if (!option.use_convertor)
        return x;
//    return x / 52.5;
    return std::min(std::max((x + 52.5) / 105.0, 0.0), 1.0);
}

double OffensiveDataExtractor::convertor_y(double y) {
    if (!option.use_convertor)
        return y;
//    return y / 34.0;
    return std::min(std::max((y + 34) / 68.0, 0.0), 1.0);
}

double OffensiveDataExtractor::convertor_dist(double dist) {
    if (!option.use_convertor)
        return dist;
//    return dist / 63.0 - 1.0;
    return dist / 123.0;
}

double OffensiveDataExtractor::convertor_dist_x(double dist) {
    if (!option.use_convertor)
        return dist;
//    return dist / 63.0 - 1.0;
    return dist / 105.0;
}

double OffensiveDataExtractor::convertor_dist_y(double dist) {
    if (!option.use_convertor)
        return dist;
//    return dist / 63.0 - 1.0;
    return dist / 68.0;
}

double OffensiveDataExtractor::convertor_angle(double angle) {
    if (!option.use_convertor)
        return angle;
//    return angle / 180.0;
    return (angle + 180.0) / 360.0;
}

double OffensiveDataExtractor::convertor_type(double type) {
    if (!option.use_convertor)
        return type;
//    return type / 9.0 - 1.0;
    return type / 18.0;
}

double OffensiveDataExtractor::convertor_cycle(double cycle) {
    if (!option.use_convertor)
        return cycle;
//    return cycle / 3000.0 - 1.0;
    return cycle / 6000.0;
}

double OffensiveDataExtractor::convertor_bv(double bv) {
    if (!option.use_convertor)
        return bv;
//    return bv / 3.0 * 2 - 1;
    return bv / 3.0;
}

double OffensiveDataExtractor::convertor_bvx(double bvx) {
    if (!option.use_convertor)
        return bvx;
//    return bvx / 3.0;
    return (bvx + 3.0) / 6.0;
}

double OffensiveDataExtractor::convertor_bvy(double bvy) {
    if (!option.use_convertor)
        return bvy;
//    return bvy / 3.0;
    return (bvy + 3.0) / 6.0;
}

double OffensiveDataExtractor::convertor_pv(double pv) {
    if (!option.use_convertor)
        return pv;
    return pv / 1.5;
}

double OffensiveDataExtractor::convertor_pvx(double pvx) {
    if (!option.use_convertor)
        return pvx;
//    return pvx / 1.5;
    return (pvx + 1.5) / 3.0;
}

double OffensiveDataExtractor::convertor_pvy(double pvy) {
    if (!option.use_convertor)
        return pvy;
//    return pvy / 1.5;
    return (pvy + 1.5) / 3.0;
}

double OffensiveDataExtractor::convertor_unum(double unum) {
    if (!option.use_convertor)
        return unum;
    if (unum == -1)
        return unum;
    return unum / 11.0;
}

double OffensiveDataExtractor::convertor_card(double card) {
    if (!option.use_convertor)
        return card;
    return card / 2.0;
}

double OffensiveDataExtractor::convertor_stamina(double stamina) {
    if (!option.use_convertor)
        return stamina;
    return stamina / 8000.0;
}

double OffensiveDataExtractor::convertor_counts(double count) {
    count = std::min(count, 20.0);
    if (!option.use_convertor)
        return count;
    return count / 20; // TODO I Dont know the MAX???
}

uint OffensiveDataExtractor::find_unum_index(DEState &state, uint unum) {
    auto players = sort_players3(state);
    if (players.size() < 11)
        std::cout<<state.kicker()->unum()<<" "<<"size problems"<<players.size()<<std::endl;
    for (uint i = 0; i < 11; i++) {
        auto player = players[i];
        if (player == nullptr)
            continue;
        if (player->unum() == unum)
            return i + 1; // TODO add 1 or not??
    }

    std::cout<<state.kicker()->unum()<<" "<<"not match"<<players.size()<<std::endl;
    return 0;
}


ODEPolar::ODEPolar(rcsc::Vector2D p) {
    teta = p.th().degree();
    r = p.r();
}
