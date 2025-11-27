#include <bits/stdc++.h>
using namespace std;

struct State {
    int r, c;
    vector<pair<int,int>> visitedGoals;
    vector<string> actions;
    int g; // هزینه واقعی
    int time;
    int f; // g + h
    bool operator>(const State& other) const {
        return f > other.f;
    }
};

int rows, cols;
vector<string> grid;
set<pair<int,int>> goals;

map<string, pair<int,int>> MOVES = {
    {"UP", {-1,0}}, {"DOWN", {1,0}}, {"LEFT", {0,-1}},
    {"RIGHT", {0,1}}, {"STAY", {0,0}}
};

ofstream logFile("log.txt");

pair<int,int> analyze_cell(char cell, int current_time) {
    if(cell=='S'||cell=='G') return {1,0};
    if(cell=='L') {
        int cycle = current_time % 20;
        if(cycle<10) return {1,0};
        else return {1, 20-cycle};
    }
    return {cell-'0',0};
}

int heuristic(int r, int c, const vector<pair<int,int>> &visited){
    int maxDist=0;
    for(auto &g : goals){
        bool done=false;
        for(auto &v: visited) if(v==g){ done=true; break;}
        if(!done){
            int d = abs(r-g.first)+abs(c-g.second);
            maxDist=max(maxDist,d);
        }
    }
    return maxDist;
}

bool is_goal(State &s){
    return s.visitedGoals.size()==goals.size();
}

void informed_search_with_log() {
    pair<int,int> start={-1,-1};
    for(int r=0;r<rows;r++)
        for(int c=0;c<cols;c++){
            if(grid[r][c]=='S') start={r,c};
            if(grid[r][c]=='G') goals.insert({r,c});
        }

    priority_queue<State, vector<State>, greater<State>> pq;
    State init={start.first,start.second,{}, {}, 0,0,0};
    init.f = init.g + heuristic(init.r,init.c,init.visitedGoals);
    pq.push(init);
    logFile << "PUSH: (" << init.r << "," << init.c << ") g=" << init.g << " time=" << init.time << "\n";

    set<tuple<int,int,vector<pair<int,int>>>> visitedSet;

    while(!pq.empty()){
        State cur = pq.top(); pq.pop();
        logFile << "POP: (" << cur.r << "," << cur.c << ") g=" << cur.g << " time=" << cur.time << "\n";

        auto state_id = make_tuple(cur.r, cur.c, cur.visitedGoals);
        if(visitedSet.count(state_id)) continue;
        visitedSet.insert(state_id);

        if(grid[cur.r][cur.c]=='G'){
            bool already=false;
            for(auto &p: cur.visitedGoals) if(p.first==cur.r && p.second==cur.c){already=true; break;}
            if(!already) cur.visitedGoals.push_back({cur.r, cur.c});
            sort(cur.visitedGoals.begin(), cur.visitedGoals.end());
        }

        if(is_goal(cur)){
            cout << "Cost: " << cur.g << " min\n";
            cout << "Actions: [";
            for(size_t i=0;i<cur.actions.size();++i){
                cout << "'" << cur.actions[i] << "'";
                if(i+1<cur.actions.size()) cout << ", ";
            }
            cout << "]\n";
            cout << "States expanded: " << visitedSet.size() << "\n";
            logFile << "Total states expanded: " << visitedSet.size() << "\n";
            return;
        }

        for(auto &mv : MOVES){
            string mvName=mv.first;
            int nr=cur.r+mv.second.first;
            int nc=cur.c+mv.second.second;
            if(nr<0 || nr>=rows || nc<0 || nc>=cols) continue;

            char cell = grid[nr][nc];
            auto [entry_cost, wait_time] = analyze_cell(cell, cur.time);

            State prev = cur;
            // STAY دقیقه‌ای برای چراغ قرمز
            for(int w=0; w<wait_time; ++w){
                State stayState = prev;
                stayState.actions.push_back("STAY");
                stayState.g += 1;
                stayState.time += 1;
                pq.push(stayState);
                logFile << "PUSH: (" << stayState.r << "," << stayState.c << ") g=" 
                        << stayState.g << " time=" << stayState.time << "\n";
                prev = stayState;
            }

            // حرکت نهایی به خانه
            State next = prev;
            next.r = nr; next.c = nc;
            next.actions.push_back(mvName);
            next.g += entry_cost;
            next.time += entry_cost;
            next.f = next.g + heuristic(next.r,next.c,next.visitedGoals);
            pq.push(next);
            logFile << "PUSH: (" << next.r << "," << next.c << ") g=" 
                    << next.g << " time=" << next.time << "\n";
        }
    }
}

int main(){
    cin >> rows >> cols;
    cin.ignore();
    grid.resize(rows);
    for(int i=0;i<rows;i++) getline(cin, grid[i]);

    informed_search_with_log();
    logFile.close();
    return 0;
}
