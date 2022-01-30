#include <iostream>
#include <fstream>
#include <array>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <limits.h>
#include <map>
#define MAXDEPTH 6
using namespace std;
struct Point {
	int x, y;
	Point() : Point(0, 0) {}
	Point(int x, int y) : x(x), y(y) {}
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
int player;
const int SIZE = 8;
std::array<std::array<int, SIZE>, SIZE> board_new;
std::vector<Point> next_valid_spots_new;
std::array<array<int, 8>, 8> weightmap{ {
		{1200, -500, 150,  50,  50, 150, -500,  1200},
		{-500, -600, -100, -100, -100, -100, -600, -500},
		{150,  -100, 50,   0,   0, 50,  -100,  150},
		{100,  -50,   0,   0,   0,   0,  -50,   100},
		{100,  -50,   0,   0,   0,   0,  -50,   100},
		{150,  -100, 50,   0,   0,  50,  -100,  150},
		{-500, -600, -100, -100, -100, -100, -600, -500},
		{1200, -500, 150,  100,  100, 150, -500,  1200},
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
	const std::array<Point, 8> directions{ {
		Point(-1, -1), Point(-1, 0), Point(-1, 1),
		Point(0, -1), /*{0, 0}, */Point(0, 1),
		Point(1, -1), Point(1, 0), Point(1, 1)
	} };
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
		for (Point dir : directions) {
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
		for (Point dir : directions) {
			// Move along the direction while testing.
			Point p = center + dir;
			if (!is_disc_at(p, get_next_player(cur_player)))
				continue;
			std::vector<Point> discs({ p });
			p = p + dir;
			while (is_spot_on_board(p) && get_disc(p) != EMPTY) {
				if (is_disc_at(p, cur_player)) {
					for (Point s : discs) {
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

	OthelloBoard(const OthelloBoard &rhs) {
		this->cur_player = rhs.cur_player;
		for (int i = 0; i < SIZE; i++) {
			for (int j = 0; j < SIZE; j++) {
				this->board[i][j] = rhs.board[i][j];
			}
		}
		this->next_valid_spots = rhs.next_valid_spots;
		done = rhs.done;
		disc_count[EMPTY] = rhs.disc_count[EMPTY];
		disc_count[EMPTY] = rhs.disc_count[BLACK];
		disc_count[WHITE] = rhs.disc_count[WHITE];
		winner = rhs.winner;
	}

	OthelloBoard() {
		reset();
	}
	void reset() {
		for (int i = 0; i < SIZE; i++) {
			for (int j = 0; j < SIZE; j++) {
				board[i][j] = EMPTY;
			}
		}
		board[3][4] = board[4][3] = BLACK;
		board[3][3] = board[4][4] = WHITE;
		cur_player = BLACK;
		disc_count[EMPTY] = 8 * 8 - 4;
		disc_count[BLACK] = 2;
		disc_count[WHITE] = 2;
		next_valid_spots = get_valid_spots();
		done = false;
		winner = -1;
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
		if (!is_spot_valid(p)) {
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
	int setHeuristic() {//temporary set the heuristic in constant.
		//if you ensure that you will win, then you can try your best do this perform.
		int heuristic = 0;
		if (!disc_count[EMPTY] && disc_count[player] > disc_count[3 - player])heuristic += 5000;
		int n = 8;
		for (int i = 0; i < n; i++) {
			for (int j = 0; j < n; j++) {
				if (board[i][j] == player)
					heuristic += weightmap[i][j];
				else if (board[i][j] == 3 - player) {
					heuristic -= weightmap[i][j];
				}
			}
		}
		//to compromise what you do in the weightmap.
		if (board[0][0] == player && board[1][1] == player) {
			if (board[1][1] == player) {
				heuristic += 500;
			}
			if (board[1][0]==player || board[0][1]==player) {
				heuristic += 300;
			}
		}
		else if (board[0][n - 1] == player ) {
			if (board[1][n - 2] == player)heuristic += 600;
			if (board[1][n - 1] == player || board[0][n - 2] == player)heuristic += 500;
		}
		else if (board[n - 1][0] == player ) {
			if (board[n - 2][1] == player)heuristic += 600;
			if (board[n - 2][0] == player || board[n - 1][1] == player)heuristic += 500;
		}
		else if (board[n - 1][n - 1] == player) {
			if (board[n - 2][n - 2] == player)heuristic += 600;
			if (board[n - 2][n - 1] == player || board[n - 1][n - 2] == player)heuristic += 500;
		}
		else if (board[0][0] == 3- player && board[1][1] ==3-  player) {
			if (board[1][1] == 3-player) {
				heuristic -= 500;
			}
			if (board[1][0] == 3-player || board[0][1] == 3-player) {
				heuristic -= 300;
			}
		}
		else if (board[0][n - 1] == 3-player) {
			if (board[1][n - 2] == 3-player)heuristic -= 600;
			if (board[1][n - 1] == 3-player || board[0][n - 2] == 3-player)heuristic -= 500;
		}
		else if (board[n - 1][0] == 3-player) {
			if (board[n - 2][1] == 3-player)heuristic -= 600;
			if (board[n - 2][0] == 3-player || board[n - 1][1] == 3-player)heuristic -= 500;
		}
		else if (board[n - 1][n - 1] == 3-player) {
			if (board[n - 2][n - 2] == 3-player)heuristic -= 600;
			if (board[n - 2][n - 1] == 3-player || board[n - 1][n - 2] == 3-player)heuristic -= 500;
		}
		//the opponents selection will effects the result.
		int size_of_next_valid_move = next_valid_spots.size();
		if (64 - disc_count[EMPTY] < 16) size_of_next_valid_move *= 2;
		if (cur_player == player) {
			heuristic += size_of_next_valid_move * 50;
		}
		else if (cur_player == 3 - player) heuristic -= size_of_next_valid_move * 100;
		//the more on the edge, the more effect the result will be.
		int cnt_enemy=0;
		int cnt = 0;
		for (int i = 0; i < n; i++) {
			for (int j = 0; j < n; j++) {
				if (i == 0||j==n-1 || j == 0 || i==0) {
					if (board[i][j] == 3 - player) {
						cnt_enemy++;
					}
					else if (board[i][j] == player) {
						cnt++;
					}
					//add the number to comprimise.
					/*if ((i == 0 || i == n - 1) && (j == 0 || j == n - 1)) {
						if (board[i][j] == 3 - player)cnt_enemy+=5;
					}*/
				}
			}
		}
		heuristic -= (cnt_enemy * 200);
		heuristic += cnt * 150;
		return heuristic;
		
	}
};

//no change
void read_board(std::ifstream& fin) {
	fin >> player;
	for (int i = 0; i < SIZE; i++) {
		for (int j = 0; j < SIZE; j++) {
			fin >> board_new[i][j];
		}
	}
}
//no change
void read_valid_spots(std::ifstream& fin) {
	int n_valid_spots;
	fin >> n_valid_spots;
	int x, y;
	for (int i = 0; i < n_valid_spots; i++) {
		fin >> x >> y;
		next_valid_spots_new.push_back({ x, y });
	}
}

//Point record;//use to record the last minimum/maximum next_spots.
map<int, Point> record;
Point record2;
//Point record;
int search_minimax(OthelloBoard now, int depth, int Alpha, int Beta) {

	if (depth == 0 || now.done) {
		return now.setHeuristic();
	}
	
	if (now.cur_player == player) {//suppose is the same(so the heuristic will be maximum)
		int maxEval = -INT_MAX;
		//catch the child out
		for (auto test : now.next_valid_spots) {
			OthelloBoard next(now);
			if (next.put_disc(test)) {
				int eval = search_minimax(next, depth - 1, Alpha, Beta);
				if (eval > maxEval && depth==MAXDEPTH) record2 = test;
				//if (eval > maxEval && depth == MAXDEPTH)cout << record2.x << " " << record2.y << endl;
				maxEval = max(maxEval, eval);
				Alpha = max(Alpha, eval);
				if (Beta <= Alpha) break;
			}
		}
		return maxEval;
	}
	else {//suppose is different
		int minEval = INT_MAX;
		for (auto test : now.next_valid_spots) {
			OthelloBoard next(now);
			if (next.put_disc(test)) {
				int eval = search_minimax(next, depth - 1, Alpha, Beta);
				if (eval < minEval && depth==MAXDEPTH)record2 = test;
				//if (eval < minEval && depth == MAXDEPTH)cout << record2.x << " " << record2.y << endl;k
				minEval = min(minEval, eval);
				Beta = min(minEval, eval);
				if (Beta <= Alpha)break;
			}
		}
		return minEval;
	}

}

void write_valid_spot(std::ofstream& fout) {
	OthelloBoard curState;
	curState.reset();
	//int n_valid_spots = next_valid_spots_new.size();
	for (int i = 0; i < SIZE; i++) {
		for (int j = 0; j < SIZE; j++) {
			curState.board[i][j] = board_new[i][j];
		}
	}
	curState.cur_player = player;
	curState.next_valid_spots = next_valid_spots_new;

	Point p;
	search_minimax(curState, MAXDEPTH, -INT_MAX, INT_MAX);
	p = record2;
	// Remember to flush the output to ensure the last action is written to file.//¦nªº
	fout << p.x << " " << p.y << std::endl;
	fout.flush();
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