#include <iostream>
#include <fstream>
#include <array>
#include <vector>
#include <cstdlib>
#include <ctime>
#define minus_INF -999999
#define pos_INF 999999

int cur_biggest=minus_INF;
int cur_index=-1;
struct Point {
    int x, y;
    Point() : Point(0, 0) {}
	Point(float x, float y) : x(x), y(y) {}
	bool operator==(const Point& rhs) const {
		return x == rhs.x && y == rhs.y;
	}
	bool operator!=(const Point& rhs) const {
		return !operator==(rhs);
	}
	Point operator+(const Point& rhs) const {
		return Point(x + rhs.x, y + rhs.y);
	}
	Point operator-(const Point& rhs) const {
		return Point(x - rhs.x, y - rhs.y);
	}
};

class OthelloBoard {
public:
    enum SPOT_STATE {
        EMPTY = 0,
        BLACK = 1,
        WHITE = 2
    };
    static const int SIZE = 8;
    const std::array<Point, 8> directions{{
        Point(-1, -1), Point(-1, 0), Point(-1, 1),
        Point(0, -1), /*{0, 0}, */Point(0, 1),
        Point(1, -1), Point(1, 0), Point(1, 1)
    }};
    const std::array<Point,4> stones{{
        Point(0,0), Point(0,7),Point(7,0), 
        Point(7,7)
    }};
    const std::array<int,5> regions{
        1000, 100, 20000, 1, 100000
    };
    const std::array<Point,12> bdPoints{{
        Point(0,1), Point(1,0),Point(1,1), 
        Point(6,0), Point(7,1),Point(6,1),
        Point(0,6), Point(1,7),Point(1,6),
        Point(7,6), Point(6,6),Point(6,7)
    }};
    /*
    const std::array<Point,12> gdPoints{{
        Point(0,2), Point(2,0),Point(2,2), 
        Point(5,0), Point(7,2),Point(5,2),
        Point(0,5), Point(2,7),Point(2,5),
        Point(7,5), Point(5,5),Point(5,7)
    }};*/
    std::array<std::array<int, SIZE>, SIZE> board;
    std::vector<Point> next_valid_spots;
    std::array<int, 3> disc_count;
    int cur_player;
    bool done;
    int winner;
private:
    int get_next_player(int player) const {
        return 3 - player;
    }
    bool is_spot_on_board(Point p) const {
        return 0 <= p.x && p.x < SIZE && 0 <= p.y && p.y < SIZE;
    }
    int get_disc(Point p) const {
        return board[p.x][p.y];
    }
    void set_disc(Point p, int disc) {
        board[p.x][p.y] = disc;
    }
    bool is_disc_at(Point p, int disc) const {
        if (!is_spot_on_board(p))
            return false;
        if (get_disc(p) != disc)
            return false;
        return true;
    }
    bool is_spot_valid(Point center) const {
        if (get_disc(center) != EMPTY)
            return false;
        for (Point dir: directions) {
            // Move along the direction while testing.
            Point p = center + dir;
            if (!is_disc_at(p, get_next_player(cur_player)))
                continue;
            p = p + dir;
            while (is_spot_on_board(p) && get_disc(p) != EMPTY) {
                if (is_disc_at(p, cur_player))
                    return true;
                p = p + dir;
            }
        }
        return false;
    }
    void flip_discs(Point center) {
        for (Point dir: directions) {
            // Move along the direction while testing.
            Point p = center + dir;
            if (!is_disc_at(p, get_next_player(cur_player)))
                continue;
            std::vector<Point> discs({p});
            p = p + dir;
            while (is_spot_on_board(p) && get_disc(p) != EMPTY) {
                if (is_disc_at(p, cur_player)) {
                    for (Point s: discs) {
                        set_disc(s, cur_player);
                    }
                    disc_count[cur_player] += discs.size();
                    disc_count[get_next_player(cur_player)] -= discs.size();
                    break;
                }
                discs.push_back(p);
                p = p + dir;
            }
        }
    }
public:
    OthelloBoard(std::array<std::array<int, SIZE>, SIZE> init_board, int Player, int disc_empty, int disc_black, int disc_white) {
        board = init_board;
        cur_player = Player;
        disc_count[EMPTY] = disc_empty;
        disc_count[BLACK] = disc_black;
        disc_count[WHITE] = disc_white;
        next_valid_spots = get_valid_spots();
        done = false;
        winner = -1;
    }
    OthelloBoard(std::array<std::array<int, SIZE>, SIZE> init_board, int Player, int disc_empty, int disc_black, int disc_white, std::vector<Point> next_valid_spots1) {
        board = init_board;
        cur_player = Player;
        disc_count[EMPTY] = disc_empty;
        disc_count[BLACK] = disc_black;
        disc_count[WHITE] = disc_white;
        next_valid_spots = next_valid_spots1;
        done = false;
        winner = -1;
    }
    OthelloBoard(const OthelloBoard& obj_board){
        board = obj_board.board;
        cur_player = obj_board.cur_player;
        disc_count[EMPTY] = obj_board.disc_count[EMPTY];
        disc_count[BLACK] = obj_board.disc_count[BLACK];
        disc_count[WHITE] = obj_board.disc_count[WHITE];
        next_valid_spots = obj_board.next_valid_spots;
        done = obj_board.done;
        winner = obj_board.winner;
    }
    /*void reset() {
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                board[i][j] = EMPTY;
            }
        }
        board[3][4] = board[4][3] = BLACK;
        board[3][3] = board[4][4] = WHITE;
        cur_player = BLACK;
        disc_count[EMPTY] = 8*8-4;
        disc_count[BLACK] = 2;
        disc_count[WHITE] = 2;
        next_valid_spots = get_valid_spots();
        done = false;
        winner = -1;
    }*/
    int evaluate_state_value(int player){
        if(!done){
            int player_score=0;
            int opponent_score=0;
            for(int i=0;i<8;i++){
                for(int j=0;j<8;j++){
                    if(board[i][j]==EMPTY)
                        continue;
                    int value=0;
                    Point pt(i,j);
                    if((i==0&&j==0)||(i==7&&j==7)||(i==0&&j==7)||(i==7&&j==0)){
                        value = regions[4];
                    }
                    else if((i==0&&j>=2&&j<=5)||(j==0&&i>=2&&i<=5)||(i==7&&j>=2&&j<=5)||(j==7&&i>=2&&i<=5)){
                        value = regions[2];
                    }
                    else if((i==1&&j>=2&&j<=5)||(j==1&&i>=2&&i<=5)||(i==6&&j>=2&&j<=5)||(j==6&&i>=2&&i<=5)){
                        value = regions[1];
                    }
                    else if(pt==bdPoints[0]||pt==bdPoints[1]||pt==bdPoints[2]||pt==bdPoints[3]||pt==bdPoints[4]||pt==bdPoints[5]||pt==bdPoints[6]||pt==bdPoints[7]||\
                    pt==bdPoints[8]||pt==bdPoints[9]||pt==bdPoints[10]||pt==bdPoints[11]){
                        value=regions[3];
                    }
                    else{
                        value=regions[0];
                    }
                    if(board[i][j]==player){
                        player_score+=value;
                    }
                    else if(board[i][j]==3-player){
                        opponent_score+=value;
                    }
                }
            }
            if(player==cur_player){
                player_score+=2000*next_valid_spots.size();
                /*for(int i=0;i<next_valid_spots.size();i++){
                    for(auto j :stones){
                        if(next_valid_spots[i]==j){
                            player_score+=100000;
                            break;
                        }
                    }
                }*/
            }
            else if(3-player==cur_player){
                opponent_score+=2000*next_valid_spots.size();
                /*for(int i=0;i<next_valid_spots.size();i++){
                    for(auto j :stones){
                        if(next_valid_spots[i]==j){
                            player_score+=100000;
                            break;
                        }
                    }
                }*/
            }
            return player_score-opponent_score;
            
            /*if(cur_player!=player){
                int ret=(30-next_valid_spots.size())*2+(disc_count[player]-disc_count[3-player]);
                for(int i=0;i<next_valid_spots.size();i++){
                    for(auto j:stones){
                        if(j==next_valid_spots[i]){
                            ret-=200;
                            break;
                        }
                    }
                    
                }
                for(auto i:stones){
                    if(get_disc(i)==player){
                        ret+=200;
                    }
                    else if(get_disc(i)==3-player){
                        ret-=200;
                    }
                }*/
                /*for(int i=0;i<12;i++){
                    if(get_disc(gdPoints[i])==player&&get_disc(bdPoints[i])==3-player){
                        ret+=30;
                    }
                    else if(get_disc(gdPoints[i])==3-player&&get_disc(bdPoints[i])==player){
                        ret-=30;
                    }
                }*/
                /*if(disc_count[player]+disc_count[3-player]<=12){
                    for(auto i:midPoints){
                        if(get_disc(i)==player)
                            ret+=100;
                        if(get_disc(i)==3-player)
                            ret-=100;
                        for(int j=0;j<next_valid_spots.size();j++){
                            if(i==next_valid_spots[j]){
                                ret-=100;
                                break;
                            }
                        }
                    }   
                }*/
                /*
                return ret;
            }
            else{
                int ret=(30+next_valid_spots.size())*2+(disc_count[player]-disc_count[3-player]);
                for(int i=0;i<next_valid_spots.size();i++){
                    for(auto j:stones){
                        if(j==next_valid_spots[i]){
                            ret+=200;
                        }
                    }
                }
                for(auto i:stones){
                    if(get_disc(i)==player){
                        ret+=200;
                    }
                    else if(get_disc(i)==3-player){
                        ret-=200;
                    }
                }*/
                /*for(int i=0;i<12;i++){
                    if(get_disc(gdPoints[i])==player&&get_disc(bdPoints[i])==3-player){
                        ret+=30;
                    }
                    else if(get_disc(gdPoints[i])==3-player&&get_disc(bdPoints[i])==player){
                        ret-=30;
                    }
                }*/
                /*if(disc_count[player]+disc_count[3-player]<=12){
                    for(auto i:midPoints){
                        if(get_disc(i)==player)
                            ret+=100;
                        if(get_disc(i)==3-player)
                            ret-=100;
                        for(int j=0;j<next_valid_spots.size();j++){
                            if(i==next_valid_spots[j]){
                                ret+=100;
                                break;
                            }
                        }
                    }   
                }*//*
                return ret;
            }*/
        }
        else{
            if(winner==player||winner==EMPTY){
                return pos_INF;
            }
            else{
                return minus_INF;
            }
        }
    }
    std::vector<Point> get_valid_spots() const {
        std::vector<Point> valid_spots;
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                Point p = Point(i, j);
                if (board[i][j] != EMPTY)
                    continue;
                if (is_spot_valid(p))
                    valid_spots.push_back(p);
            }
        }
        return valid_spots;
    }
    bool put_disc(Point p) {
        if(!is_spot_valid(p)) {
            winner = get_next_player(cur_player);
            done = true;
            return false;
        }
        set_disc(p, cur_player);
        disc_count[cur_player]++;
        disc_count[EMPTY]--;
        flip_discs(p);
        // Give control to the other player.
        cur_player = get_next_player(cur_player);
        next_valid_spots = get_valid_spots();
        // Check Win
        if (next_valid_spots.size() == 0) {
            cur_player = get_next_player(cur_player);
            next_valid_spots = get_valid_spots();
            if (next_valid_spots.size() == 0) {
                // Game ends
                done = true;
                int white_discs = disc_count[WHITE];
                int black_discs = disc_count[BLACK];
                if (white_discs == black_discs) winner = EMPTY;
                else if (black_discs > white_discs) winner = BLACK;
                else winner = WHITE;
            }
        }
        return true;
    }
    
};
int alphabeta(OthelloBoard board, int depth, int alpha, int beta, bool maximizing,int head_idx,std::ofstream& fout);
int player;
const int SIZE = 8;
std::array<std::array<int, SIZE>, SIZE> board1;
std::vector<Point> next_valid_spots1;

void read_board(std::ifstream& fin) {
    fin >> player;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            fin >> board1[i][j];
        }
    }
}

void read_valid_spots(std::ifstream& fin) {
    int n_valid_spots;
    fin >> n_valid_spots;
    int x, y;
    for (int i = 0; i < n_valid_spots; i++) {
        fin >> x >> y;
        next_valid_spots1.push_back({x, y});
    }
}

void write_valid_spot(std::ofstream& fout) {
    int n_valid_spots = next_valid_spots1.size();
    //srand(time(NULL));
    // Choose random spot. (Not random uniform here)
    //int index = (rand() % n_valid_spots);
    const std::array<Point,4> stone{{
        Point(0,0), Point(0,7),Point(7,0), 
        Point(7,7)
    }};
    for(int i=0;i<n_valid_spots;i++){
        for(auto j:stone){
            if(next_valid_spots1[i]==j){
                Point p = next_valid_spots1[i];
                fout << p.x << " " << p.y << std::endl;
                fout.flush();
                return;
            }
        }
    }
    std::vector<int> stateValue;
    int cnt_empty=0;
    int cnt_black=0;
    int cnt_white=0;
    for(int i=0;i<8;i++){
        for(int j=0;j<8;j++){
            if(board1[i][j]==0)
                cnt_empty++;
            else if(board1[i][j]==1)
                cnt_black++;
            else if(board1[i][j]==2)
                cnt_white++;
        }
    }
    for(int i=0;i<n_valid_spots;i++){
        OthelloBoard cur_board(board1, player, cnt_empty, cnt_black, cnt_white, next_valid_spots1);
        cur_board.put_disc(next_valid_spots1[i]);
        stateValue.push_back(alphabeta(cur_board, 3,minus_INF,pos_INF,false,i,fout));
    }
    int index=-1;
    int biggest=minus_INF-1;
    for(int i=0;i<stateValue.size();i++){
        if(stateValue[i]>biggest){
            biggest=stateValue[i];
            index=i;
        }
        //std::cout<<next_valid_spots1[i].x<<" "<<next_valid_spots1[i].y<<std::endl;
        //std::cout<<stateValue[i]<<std::endl;
    }
    Point p = next_valid_spots1[index];

    // Remember to flush the output to ensure the last action is written to file.
    fout << p.x << " " << p.y << std::endl;
    fout.flush();
}

int alphabeta(OthelloBoard board, int depth, int alpha, int beta, bool maximizing,int head_idx,std::ofstream& fout){
    if(depth==0||board.done){
        int ret=board.evaluate_state_value(player);
        if(ret>cur_biggest){
            cur_biggest=ret;
            cur_index=head_idx;
            Point p = next_valid_spots1[cur_index];
            fout << p.x << " " << p.y << std::endl;
            fout.flush();
        }
        return ret;
    }
    if(maximizing==true){
        int value = minus_INF;
        for(int i=0;i<board.next_valid_spots.size();i++){
            OthelloBoard next_board(board);
            if(next_board.put_disc(board.next_valid_spots[i])){
                value = std::max(value, alphabeta(next_board, depth-1, alpha, beta, false,head_idx,fout));
                alpha = std::max(alpha, value);
                if(alpha>=beta)
                    break;
            }
        }
        return value;
    }
    else{
        int value = pos_INF;
        for(int i=0;i<board.next_valid_spots.size();i++){
            OthelloBoard next_board(board);
            if(next_board.put_disc(board.next_valid_spots[i])){
                value = std::min(value, alphabeta(next_board, depth-1, alpha, beta, true,head_idx,fout));
                beta = std::min(beta, value);
                if(beta<=alpha)
                    break;
            }
        }
        return value;
    }
}

int main(int, char** argv) {
    std::ifstream fin(argv[1]);
    std::ofstream fout(argv[2]);
    read_board(fin);
    read_valid_spots(fin);
    write_valid_spot(fout);
    fin.close();
    fout.close();
    return 0;
}
