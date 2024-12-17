import subprocess
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.ticker import MaxNLocator
import csv

exe_path = "C:\\Users\\xxxxx\\source\\repos\\Algorithm\\x64\Debug\\"
exe_name = exe_path + "Algorithm.exe"

out_path = "C:\\Users\\xxxxx\\source\\repos\\Algorithm\\";

# データ送受信のプロトコル：
# 1. pythonからのc++起動指示は以下の通り。
#     ****.exe [boardsize] [CPUstrength1] [CPUstrength2] [testcount]
# 2. c++からのテスト結果の出力ファイル名は以下の通り。
#     output_[boardsize]_[CPUstrength1]_[CPUstrength2]_[testcount].csv
# 3. c++からのテスト結果の出力形式は以下の通り。
#     [winner(1or2, draw==0)],[CPU1score],[CPU2score] * testcount
# 4. pythonからのvisualize結果の出力ファイル名は以下の通り。
#     output_[boardsize]_[CPUstrength1]_[CPUstrength2]_[testcount]_visualize.png
# 5. pythonからの最終まとめ結果の出力ファイル名は以下の通り。
#     output_[testcount].csv
# 6. pythonからの最終まとめ結果の出力形式は以下の通り。
#     [boardsize],[CPU1strength],[CPU2strength],[testcount],[win1_count],[win2_count],[win1_rate],[win2_rate]


# テストを実行して結果をoutFileに格納する関数
def execTest(cmd_string, testcount):
    proc = subprocess.Popen(cmd_string, shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)

    while proc.poll() is None: #まだ実行中ならNone
        line = proc.stdout.readline().decode('utf8', 'replace')
        if line:
            dispstr = "case " + line.replace('\n', '').replace('\r', '') + " / " + str(testcount) 
            print("\r" + dispstr, end="", flush=True)
    print("")
    print("test completed")
    

# outFileからデータを読み込んでリストを返す関数
def load_outFile(output_csvname):
    records = []
    with open(output_csvname, 'r') as file:
        for line in file:
            parts = line.strip().split(',')
            if len(parts) == 3:
                try:
                    winner = int(parts[0].strip())
                    score1 = int(parts[1].strip())
                    score2 = int(parts[2].strip())
                    records.append([winner, score1, score2])
                except ValueError:
                    continue
    return records

# 最終まとめ結果からデータを読み込んでリストを返す関数
def load_TotalResultFile(output_totalresult_name):
    records = []
    with open(output_totalresult_name, 'r') as file:
        for line in file:
            parts = line.strip().split(',')
            if len(parts) == 8:
                try:
                    boardsize = int(parts[0].strip())
                    CPUstrength1 = int(parts[1].strip())
                    CPUstrength2 = int(parts[2].strip())
                    testcount = int(parts[3].strip())
                    win1_count = int(parts[4].strip())
                    win2_count = int(parts[5].strip())
                    win1_rate = float(parts[6].strip())
                    win2_rate = float(parts[7].strip())
                    records.append([boardsize, CPUstrength1, CPUstrength2, testcount, win1_count, win2_count, win1_rate, win2_rate])
                except ValueError:
                    continue
    return records

# 勝ち数と勝率を計算するヘルパー関数
def calcWinCounts(records):
    testcount = len(records)
    win1_count = sum(1 for r in records if r[0] == 1)
    win2_count = sum(1 for r in records if r[0] == 2)
    win1_rate = win1_count / testcount * 100
    win2_rate = win2_count / testcount * 100
    return win1_count, win2_count, win1_rate, win2_rate


# 結果をヒストグラムに可視化する関数
def visualize(records, output_figname, CPUstrength1, CPUstrength2, needDisplay):
    diffs = [r[1] - r[2] for r in records]
    testcount = len(records)
    _, _, win1_rate, win2_rate = calcWinCounts(records)

    bins = np.arange(-50, 50) - 0.5
    counts, bin_edges = np.histogram(diffs, bins=bins)

    colors = ["deepskyblue" if diff > 0 else "tomato" if diff < -0.5 else "gray" for diff in bin_edges[:-1]]
    
    plt.gca().yaxis.set_major_locator(MaxNLocator(integer=True))
    plt.bar(bin_edges[:-1], counts, color=colors, width=1, edgecolor="black")
    plt.title(f"Score Differences in {testcount} iterations\nWin Rate: CPU1 (str: {CPUstrength1}) : {win1_rate:.2f}% | CPU2 (str: {CPUstrength2}) : {win2_rate:.2f}%")
    plt.xlabel("Score Difference (1st Player - 2nd Player)")
    plt.ylabel("Frequency")
    plt.grid(axis='y', linestyle='--', alpha=0.7)
    plt.tight_layout()

    plt.savefig(output_figname)
    if needDisplay:
        plt.show()
    plt.clf()
    plt.close()
    
    print("visualize completed")
    

# boardsize, CPUstrength1, CPUstrength2, testcount が決定したとき、そのパターンのテストを行う関数
def execTestAndVisualize(boardsize, CPUstrength1, CPUstrength2, testcount, output_totalresult_name=None):
    print(f"test started: boardsize={boardsize}, CPUstrength1={CPUstrength1}, CPUstrength2={CPUstrength2}, testcount={testcount}")

    cmd_string = exe_name + " " + str(boardsize) + " " + str(CPUstrength1) + " " + str(CPUstrength2) + " " + str(testcount)
    execTest(cmd_string, testcount)

    output_base_string = out_path + "output_" + str(boardsize) + "_" + str(CPUstrength1) + "_" + str(CPUstrength2) + "_" + str(testcount)
    output_csvname = output_base_string + ".csv"
    result = load_outFile(output_csvname)

    output_figname = output_base_string + "_visualize.png"
    visualize(result, output_figname, CPUstrength1, CPUstrength2, needDisplay=False)
    
    if output_totalresult_name != None:
        with open(output_totalresult_name, mode='a', newline="") as f:
            win1_count, win2_count, win1_rate, win2_rate = calcWinCounts(result)
            data = [boardsize, CPUstrength1, CPUstrength2, testcount, win1_count, win2_count, win1_rate, win2_rate]
            w = csv.writer(f)
            w.writerow(data)

    print("single test execution completed")


# 全体のテストを統括する関数
def main():
    if True:
        testcount = 500
        execTestAndVisualize(6, 99, 1, testcount)
        execTestAndVisualize(6, 1, 99, testcount)
        
    else:
        testcount = 500
        output_totalresult_name = out_path + "output_"  + str(testcount) + ".csv"

        # テストを再開するか、初めから行うかの処理。
        needTestResume = True   # 普通はFalse。テストが途中で中断され、途中から再開が必要な場合のみTrueに
        skipTargetOnTestResume = []
        if needTestResume:
            doneRecords = load_TotalResultFile(output_totalresult_name)
            for done in doneRecords:
                skipTargetOnTestResume.append([done[0], done[1], done[2], done[3]])
        else:
            with open(output_totalresult_name, mode='w') as f:
                f.write("boardsize,CPUstrength1,CPUstrength2,testcount,win1_count,win2_count,win1_rate,win2_rate\n")

        # [boardsize (4, 6)], [CPstr1 (1 ~ boardsize*boardsize)], [CPstr2 (1 ~ boardsize*boardsize)] で三重ループ
        boardsize_candidate = [4, 6]
        for boardsize in boardsize_candidate:
            CPUstrength_candidate = range(1, boardsize*boardsize + 1)
            for CPUstrength1 in CPUstrength_candidate:
                for CPUstrength2 in CPUstrength_candidate:
                    if [boardsize, CPUstrength1, CPUstrength2, testcount] in skipTargetOnTestResume:
                        continue

                    # テスト実行、結果出力
                    execTestAndVisualize(boardsize, CPUstrength1, CPUstrength2, testcount, output_totalresult_name)
     
    # 終了
    print("all tests completed!")


if __name__ == '__main__':
    main()

#先手後手を入れ替える必要がある場合のコード保管庫
#if mixturn == 1:
#    resultB = execTest(boardsize, CPUstrength2, CPUstrength1, testcount)
#    resultB = list(map(lambda r: (3 - r[0] if r[0] >= 1 else 0, r[2], r[1]), resultB)) # 勝敗とスコアを適宜反転
#    result = result + resultB