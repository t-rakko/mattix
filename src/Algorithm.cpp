#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <random>
#include <climits>
#include <string>
#include <chrono>
using namespace std;
template<typename T> bool chmin(T &a, T b) { if (a > b) { a = b; return true; } return false; }
template<typename T> bool chmax(T &a, T b) { if (a < b) { a = b; return true; } return false; }

const string outFilePath = "C:\\Users\\xxxxx\\source\\repos\\Algorithm\\";
const int CPUMaxThinkTimeInMs = 1000;

const int INF = 100000; //Mattixのスコア範囲ならこれで十分。INT_MINを使うと反転時にオーバーフローするので注意

struct Pos {
    int Row;
    int Col;
    Pos() : Row(-1), Col(-1) {}
    Pos(int _r, int _c) : Row(_r), Col(_c) {}
};
using Action = Pos;
Action ACTION_ERR = Action();

using Score = int;

struct GameSetup {
    int boardSize;
    bool player1_isCPU;
    int player1_CPUstrength;
    bool player2_isCPU;
    int player2_CPUstrength;
};

struct Player {
    string name;
    bool isCPU;
    int cpuStrength;
    Score score;
};

struct GameState {
    //盤面系の情報
    int boardSize;
    vector<vector<int>> cells;
    vector<vector<bool>> taken;
    Pos cross;

    //プレイヤー系の情報
    vector<Player> players;
    int currentPlayerID;   //0: 先手番 1: 後手番

    //盤面・プレイヤー情報を初期化する
    int initializeGame(const GameSetup &setup) {
        //ボードサイズ指定
        if (setup.boardSize != 4 && setup.boardSize != 6) return -1;
        boardSize = setup.boardSize;
        //プレイヤー情報登録
        players.resize(2);
        players[0].isCPU = setup.player1_isCPU;
        players[0].cpuStrength = setup.player1_CPUstrength;
        players[1].isCPU = setup.player2_isCPU;
        players[1].cpuStrength = setup.player2_CPUstrength;
        if (players[0].isCPU && players[1].isCPU) {
            players[0].name = "コンピュータ１";
            players[1].name = "コンピュータ２";
        }
        else if (players[0].isCPU && !players[1].isCPU) {
            players[0].name = "コンピュータ";
            players[1].name = "あなた";
        }
        else if (!players[0].isCPU && players[1].isCPU) {
            players[0].name = "あなた";
            players[1].name = "コンピュータ";
        }
        else {
            players[0].name = "プレイヤー１";
            players[1].name = "プレイヤー２";
        }
        players[0].score = 0;
        players[1].score = 0;
        //盤面初期化
        cells.resize(boardSize);
        for (int i = 0; i < (int)cells.size(); i++) cells[i].resize(boardSize);
        taken.resize(boardSize);
        for (int i = 0; i < (int)taken.size(); i++) taken[i].resize(boardSize);
        vector<int> values = boardSize == 4
            ? vector<int>{ 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8 } : vector<int>{ 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, -1, -1, -2, -2, -3, -3, -4, -5, -6, -7, -8, -9, -10 };
        std::random_device get_rand_dev;
        std::mt19937 get_rand_mt(get_rand_dev());
        shuffle(values.begin(), values.end(), get_rand_mt);
        for (int r = 0; r < boardSize; r++) {
            for (int c = 0; c < boardSize; c++) {
                int i = r * boardSize + c;
                cells[r][c] = values[i];
                if (values[i] == 0) {
                    cross.Row = r;
                    cross.Col = c;
                    taken[r][c] = true;
                }
                else {
                    taken[r][c] = false;
                }
            }
        }
        //先手番にする
        currentPlayerID = 0;
        return 0;
    }

    //現在のプレイヤー視点での盤面評価をする
    Score getScoreFromCurrentPlayerView() const {
        return players[currentPlayerID].score - players[1 - currentPlayerID].score;
    }

    //現在のプレイヤーが可能な行動をすべて取得する
    vector<Action> getValidActions() const {
        vector<Action> validActions;
        if (currentPlayerID == 0) {
            for (int c = 0; c < boardSize; c++) {
                if (!taken[cross.Row][c]) validActions.push_back({ cross.Row, c });
            }
        }
        else {
            for (int r = 0; r < boardSize; r++) {
                if (!taken[r][cross.Col]) validActions.push_back({ r, cross.Col });
            }
        }
        return validActions;
    }

    //ゲームが終了したか判定する
    bool isFinished() const {
        return getValidActions().size() == 0;
    }
    bool isDraw() const {
        return players[0].score == players[1].score;
    }
    bool isPlayer1Won() const {
        return players[0].score > players[1].score;
    }

    //指定したactionが有効な手か判定する(手動入力用)
    bool isValidAction(const Action newCross) const {
        if (newCross.Row < 0 || boardSize <= newCross.Row || newCross.Col < 0 || boardSize <= newCross.Col) return false;
        if (taken[newCross.Row][newCross.Col]) return false;
        return (currentPlayerID == 0 && cross.Row == newCross.Row) || (currentPlayerID == 1 && cross.Col == newCross.Col);
    }

    //指定したactionでゲームを1ターンすすめ、次のプレイヤーの手番にする
    void advance(const Action newCross) {
        players[currentPlayerID].score += cells[newCross.Row][newCross.Col];
        taken[newCross.Row][newCross.Col] = true;
        cross.Row = newCross.Row;
        cross.Col = newCross.Col;
        currentPlayerID = 1 - currentPlayerID;
    }
};

void displayGameStateOnCUI(const GameState &state) {
    //盤面を表示
    for (int r = 0; r < state.boardSize; r++) {
        for (int c = 0; c < state.boardSize; c++) {
            if (r == state.cross.Row && c == state.cross.Col) {
                cout << std::right << std::setw(3) << "X"; // クロスチップを表示
            }
            else if (state.taken[r][c]) {
                cout << std::right << std::setw(3) << "."; // 空のマス
            }
            else {
                cout << std::right << std::setw(3) << state.cells[r][c]; // 駒の数字
            }
        }
        cout << endl;
    }
    //スコア・手番の情報を表示
    cout << state.players[state.currentPlayerID].name << "の番です。" << endl;
    cout << state.players[0].name << "のスコア: " << state.players[0].score << endl;
    cout << state.players[1].name << "のスコア: " << state.players[1].score << endl;
}

void displayResultOnCUI(const GameState &state) {
    cout << "ゲーム終了！" << endl;
    if (state.isDraw()) {
        cout << "引き分け！" << endl;
    }
    else if (state.isPlayer1Won()) {
        cout << state.players[0].name << "の勝利！" << endl;
    }
    else {
        cout << state.players[1].name << "の勝利！" << endl;
    }
}
void outputResultOnTest(const GameState &state, const string &outFileName) {
    ofstream file(outFileName, ios::app);
    int winner = state.isDraw() ? 0 : (state.isPlayer1Won() ? 1 : 2);
    file << winner << "," << state.players[0].score << "," << state.players[1].score << endl;
}

void setupGameOnCUI(GameSetup &setup) {
    cout << "対戦形式を選択してください" << endl;
    cout << "0:人間vs人間, 1 : 人間vsCPU(弱), 2 : CPU(強)vs人間, 3 : CPU(強)vsCPU(弱)" << endl;
    int pattern;
    cin >> pattern;

    if (pattern == 0) {
        setup.boardSize = 4;
        setup.player1_isCPU = false;
        setup.player1_CPUstrength = -1;
        setup.player2_isCPU = false;
        setup.player2_CPUstrength = -1;
    } else if (pattern == 1) {
        setup.boardSize = 6;
        setup.player1_isCPU = false;
        setup.player1_CPUstrength = -1;
        setup.player2_isCPU = true;
        setup.player2_CPUstrength = 1;
    }
    else if (pattern == 2) {
        setup.boardSize = 6;
        setup.player1_isCPU = true;
        setup.player1_CPUstrength = 99;
        setup.player2_isCPU = false;
        setup.player2_CPUstrength = -1;
    }
    else {
        setup.boardSize = 6;
        setup.player1_isCPU = true;
        setup.player1_CPUstrength = 99;
        setup.player2_isCPU = true;
        setup.player2_CPUstrength = 1;
    }
}
void setupGameOnTest(const int boardsize, const int CPUstrength1, const int CPUstrength2, const int testcount, GameSetup &setup, string &outFileName) {
    setup.boardSize = boardsize;
    setup.player1_isCPU = true;
    setup.player1_CPUstrength = CPUstrength1;
    setup.player2_isCPU = true;
    setup.player2_CPUstrength = CPUstrength2;
    outFileName = outFilePath + "output_" + to_string(boardsize) + "_" + to_string(CPUstrength1) + "_" + to_string(CPUstrength2) + "_" + to_string(testcount) + ".csv";
    ofstream file(outFileName, ios::trunc);
}

Action inputActionOnCUI(const GameState &state) {
    cout << "クロスチップを" << (state.currentPlayerID == 0 ? "横" : "縦") << "に移動 (0から" << state.boardSize - 1 << ")" << endl;
    int input; 
    cin >> input;

    Action newCross;
    if (state.currentPlayerID == 0) {
        newCross.Row = state.cross.Row;
        newCross.Col = input;
    }
    else {
        newCross.Row = input;
        newCross.Col = state.cross.Col;
    }
    return newCross;
}

// 時間を管理するクラス。thun-cさんのゲームAI実装からほぼ丸コピ。
class TimeKeeper {
private:
    std::chrono::high_resolution_clock::time_point start_time_;
    int64_t time_threshold_;

public:
    // 時間制限をミリ秒単位で指定してインスタンスをつくる。
    TimeKeeper(const int64_t &time_threshold)
        :start_time_(std::chrono::high_resolution_clock::now()),
        time_threshold_(time_threshold)
    {
    }

    // インスタンス生成した時から指定した時間制限を超過したか判定する。
    bool isTimeOver() const {
        auto diff = std::chrono::high_resolution_clock::now() - this->start_time_;
        return std::chrono::duration_cast<std::chrono::milliseconds>(diff).count() >= time_threshold_;
    }

};

// スコア計算
Score negaAlphaScore(const GameState &state, Score alpha, Score beta, const int depth, const TimeKeeper &time_keeper) {
    if (time_keeper.isTimeOver()) return 0;
    if (depth == 0 || state.isFinished() || state.getValidActions().empty()) {
        return state.getScoreFromCurrentPlayerView();
    }

    for (const auto &action : state.getValidActions()) {
        GameState next_state = state;
        next_state.advance(action);
        Score score = -negaAlphaScore(next_state, -beta, -alpha, depth - 1, time_keeper);
        if (time_keeper.isTimeOver()) return 0;

        if (score > alpha) {
            alpha = score;
        }
        if (alpha >= beta) {
            //常に手番プレイヤー視点で探索することで、βカットのみ考えれば良くなる
            return alpha;
        }
    }
    return alpha;
}

// 深さ制限と制限時間(ms)を指定してNegaAlpha法で行動を決定する
// Alpha = 現在探索している盤面の、手番プレイヤー視点でのベストスコア
// Beta = 現在探索している盤面の、相手プレイヤー視点でのベストスコア
Action negaAlphaActionWithTimeThreshold(const GameState &state, const int depth, const TimeKeeper &time_keeper) {
    Action best_action = { -1, -1 };
    Score alpha = -INF;
    Score beta = INF;
    for (const auto &action : state.getValidActions()) {
        GameState next_state = state;
        next_state.advance(action);
        Score score = -negaAlphaScore(next_state, -beta, -alpha, depth - 1, time_keeper);

        //cout << "depth = " << depth << ", act = (" << action.Row << ", " << action.Col << "), Score = " << score << endl;
        if (time_keeper.isTimeOver()) return ACTION_ERR;
        if (score > alpha) {
            best_action = action;
            alpha = score;
        }
    }
    return best_action;
}

// CPUの行動決定エンジン。制限時間(ms)を指定して、反復深化で行動を決定する
Action iterativeDeepeningAction(const GameState &state, const int64_t time_threshold) {
    auto time_keeper = TimeKeeper(time_threshold);
    const int depthlimit = state.players[state.currentPlayerID].cpuStrength;

    Action best_action = { -1, -1 };
    for (int depth = 1; depth <= depthlimit; depth++) {
        if (depth > state.boardSize * state.boardSize) break;

        Action action = negaAlphaActionWithTimeThreshold(state, depth, time_keeper);

        if (time_keeper.isTimeOver()) {
            //cout << depth << endl; //NegaMax法だと初期盤面から7手先、NegaAlpha法だと初期盤面から11手先まで読める
            break;
        }
        else {
            best_action = action;
        }
    }
    return best_action;
}

// CPUの行動を、NegaMax法＋IterativeDeepeningによって探索する。探索時間は1秒。
Action calculateAction(const GameState &state) {
    return iterativeDeepeningAction(state, CPUMaxThinkTimeInMs);
}

// CUIベースのメイン処理 (View層のモック)
int mainCUI() {
    // 初期化処理
    GameSetup setup;
    setupGameOnCUI(setup);
    GameState state;
    if (state.initializeGame(setup) != 0) return -1;

    // ゲームループ
    while (true) {
        displayGameStateOnCUI(state);

        //終了条件確認
        if (state.isFinished()) break;

        //次の行動を入力or計算
        Action newCross;
        if (!state.players[state.currentPlayerID].isCPU) {
            while (true) {
                newCross = inputActionOnCUI(state);
                if (state.isValidAction(newCross)) break;
                cout << "入力ミスです。もう一度入力してください。" << endl;
            }
        }
        else {
            int st = clock();
            newCross = calculateAction(state);
            while (clock() - st < 1 * CLOCKS_PER_SEC);
        }

        //実行 (スコア計算・クロスチップ移動・プレイヤー交代)
        state.advance(newCross);
    }

    // 結果表示
    displayResultOnCUI(state);

    return 0;
}

// pythonテスト用のメイン処理
int mainTest(int boardsize, int CPUstrength1, int CPUstrength2, int testcount) {
    GameSetup setup;
    string outFileName;
    setupGameOnTest(boardsize, CPUstrength1, CPUstrength2, testcount, setup, outFileName);

    GameState state;
    for (int i = 0; i < testcount; i++) {
        cout << i + 1 << endl;

        if (state.initializeGame(setup) != 0) return -1;

        while (true) {
            if (state.isFinished()) break;
            Pos newCross = calculateAction(state);
            state.advance(newCross);
        }
        outputResultOnTest(state, outFileName);
    }

    return 0;
}

//#define __EMSCRIPTEN__ //デバッグ用
#ifndef __EMSCRIPTEN__
// メイン処理 (View層のモック)
int main(int argc, char *argv[]) {
    // 勝率調査テスト用の処理 hoge.exe [boardsize] [cpu1_strength] [cpu2_strength] [testcount]
    if (argc == 5) {
        return mainTest(stoi(argv[1]), stoi(argv[2]), stoi(argv[3]), stoi(argv[4]));
    }

    return mainCUI();
}
#endif

// Javascriptから呼び出されるときの処理
#ifdef __EMSCRIPTEN__
#include <emscripten/bind.h>
GameState JS_state;

//ラッパー関数群
//初期化
void JS_initializeGame(int boardsize, bool player1_isCPU, int player1_CPUstrength, bool player2_isCPU, int player2_CPUstrength) {
    GameSetup setup;
    setup.boardSize = boardsize;
    setup.player1_isCPU = player1_isCPU;
    setup.player1_CPUstrength = player1_CPUstrength;
    setup.player2_isCPU = player2_isCPU;
    setup.player2_CPUstrength = player2_CPUstrength;
    JS_state.initializeGame(setup);
}
//画面更新
Pos idx2pos(int chipIndex) { return Pos(chipIndex / JS_state.boardSize, chipIndex % JS_state.boardSize); }
int JS_getBoardSize() { return JS_state.boardSize; }
bool JS_isCrossChip(int chipIndex) { return idx2pos(chipIndex).Row == JS_state.cross.Row && idx2pos(chipIndex).Col == JS_state.cross.Col; }
bool JS_isTaken(int chipIndex) { return JS_state.taken[idx2pos(chipIndex).Row][idx2pos(chipIndex).Col]; }
bool JS_needFirstHighlight(int chipIndex) { return !JS_isTaken(chipIndex) && JS_state.currentPlayerID == 0 && idx2pos(chipIndex).Row == JS_state.cross.Row; }
bool JS_needSecondHighlight(int chipIndex) { return !JS_isTaken(chipIndex) && JS_state.currentPlayerID == 1 && idx2pos(chipIndex).Col == JS_state.cross.Col; }
int JS_getChipValue(int chipIndex) { return JS_state.cells[idx2pos(chipIndex).Row][idx2pos(chipIndex).Col]; }
string JS_getCurrentPlayerString() { return "現在の手番：" + JS_state.players[JS_state.currentPlayerID].name; }
string JS_getScoreString() { return JS_state.players[0].name + "：" + to_string(JS_state.players[0].score) + "点、 " + JS_state.players[1].name + "：" + to_string(JS_state.players[1].score) + "点"; }
//終了判定
bool JS_isFinished() { return JS_state.isFinished(); }
//思考ループ
Action idx2action(int chipIndex) { return Action(chipIndex / JS_state.boardSize, chipIndex % JS_state.boardSize); }
int action2idx(Action action) { return action.Row * JS_state.boardSize + action.Col; }
bool JS_isCurrentPlayerCPU() { return JS_state.players[JS_state.currentPlayerID].isCPU; }
bool JS_isValidAction(int chipIndex) { return JS_state.isValidAction(idx2action(chipIndex)); }
int JS_calculateAction() { return action2idx(calculateAction(JS_state)); }
//実行
void JS_advance(int chipIndex) { JS_state.advance(idx2action(chipIndex)); }
//ゲーム終了
string JS_getGameOverString() {
    if (JS_state.isDraw()) return "引き分け！";
    return (JS_state.isPlayer1Won() ? JS_state.players[0].name : JS_state.players[1].name) + "の勝ち！";
}

EMSCRIPTEN_BINDINGS(myModule)
{
    emscripten::function("JS_initializeGame", &JS_initializeGame);
    emscripten::function("JS_getBoardSize", &JS_getBoardSize);
    emscripten::function("JS_isCrossChip", &JS_isCrossChip);
    emscripten::function("JS_isTaken", &JS_isTaken);
    emscripten::function("JS_needFirstHighlight", &JS_needFirstHighlight);
    emscripten::function("JS_needSecondHighlight", &JS_needSecondHighlight);
    emscripten::function("JS_getChipValue", &JS_getChipValue);
    emscripten::function("JS_getCurrentPlayerString", &JS_getCurrentPlayerString);
    emscripten::function("JS_getScoreString", &JS_getScoreString);
    emscripten::function("JS_isFinished", &JS_isFinished);
    emscripten::function("JS_isCurrentPlayerCPU", &JS_isCurrentPlayerCPU);
    emscripten::function("JS_isValidAction", &JS_isValidAction);
    emscripten::function("JS_calculateAction", &JS_calculateAction);
    emscripten::function("JS_advance", &JS_advance);
    emscripten::function("JS_getGameOverString", &JS_getGameOverString);
}
#endif