#include <bits/stdc++.h>
using namespace std;

namespace Color {
enum Code {
  FG_RED     = 31,
  FG_GREEN   = 32,
  FG_YELLOW  = 33,
  FG_BLUE    = 34,
  FG_MAGENTA = 35,
  FG_CYAN    = 36,
  FG_DEFAULT = 39
};
class ColorDebugger {
  Code code;
 public:
  ColorDebugger(Code pCode) : code(pCode) {}
  template <class T>
  ColorDebugger& operator<<(const T &other) {
    // cerr << "\033[" << code << "m" << other << "\033[" << FG_DEFAULT << "m";
    cout << other;
    return *this;
  }
  ColorDebugger& operator<<(ostream & (*pManip)(ostream&)) {
    (*pManip)(cout);
    return *this;
  }
};
}

using namespace Color;
ColorDebugger rcout(FG_RED);
ColorDebugger ycout(FG_YELLOW);
ColorDebugger gcout(FG_GREEN);
ColorDebugger bcout(FG_BLUE);
ColorDebugger mcout(FG_MAGENTA);
ColorDebugger ccout(FG_CYAN);


struct PostingEntry {
  int docID;
  double score;
  PostingEntry(int _docID, double _score): docID(_docID), score(_score) { }
  // Sort by score first
  bool operator <(const PostingEntry &other)const {
    if (score == other.score) return docID < docID;
    return score < other.score;
  }

  bool operator >(const PostingEntry &other) const {
    return other < *this;
  }
};

typedef vector<PostingEntry>::iterator PostingIterator;

struct PostingsList {
  string term;
  vector <PostingEntry> *list;
  double upperBoundScore;
  int pointer;
  PostingsList (string _term, vector <PostingEntry> postingsList):
    term(_term),
    upperBoundScore(0),
    pointer(0) {
    list = new vector<PostingEntry>(postingsList);
    for ( auto &cur : (postingsList)) {
      upperBoundScore = max(upperBoundScore, cur.score);
    }
    list->push_back({INT_MAX, INT_MAX});
  }
  int size() {
    return (int) list->size();
  }
  void resetPointer () {
    pointer = 0;
  }
  void next () {
    pointer++;
  }
  void prev () {
    pointer--;
  }
  PostingEntry currentValue () {
    return (*list)[pointer];
  }
  void debug() {
    bcout << pointer << endl;
    for (const auto &cur : (*list)) {
      ycout << cur.docID << " " << cur.score << endl;
    }
    rcout << "------" << endl;
  }

};

// Memetakan dari string ke pointer of vector
map<string, PostingsList*> posting;
typedef priority_queue <PostingEntry, vector <PostingEntry>, greater<PostingEntry>> Heap;

/** Sort and recompute prefix sum in O(Q log Q) time **/
void fullSort(vector <PostingsList*> &queriedPostingList, vector <double> &prefixSum) {
  int n = queriedPostingList.size();

  sort(
    begin(queriedPostingList),
    end(queriedPostingList),
  [](PostingsList * list1, PostingsList * list2) {
    return list1->currentValue().docID < list2->currentValue().docID;
  });

  for (int i = 0; i < n; i++) {
    prefixSum[i] = (i > 0 ? prefixSum[i - 1] : 0);
    prefixSum[i] += queriedPostingList[i]->upperBoundScore;
  }
}

/** Partial Sort and readjust prefix sum in O(Q) time **/
void partialSort(vector <PostingsList*> &queriedPostingList, vector <double> &prefixSum) {
  int n = queriedPostingList.size();
  // Bubble sort 1 step, because its faster
  for (int i = 0; i < n - 1; i++) {
    auto cur = queriedPostingList[i]->currentValue();
    auto nx = queriedPostingList[i + 1]->currentValue();
    if (cur.docID > nx.docID) {
      // Adjust prefix sum here
      prefixSum[i] += nx.score - cur.score;
      swap(queriedPostingList[i], queriedPostingList[i + 1]);
    }
  }

}

// Readjust iterator of the sorted prefix so that its docID is >= k
bool readjust(vector <PostingsList*> &queriedPostingList, int k) {
  int n = queriedPostingList.size();
  bool changed = 0;
  for (int i = 0; i < n; i++) {
    if (queriedPostingList[i]->currentValue().docID >= k) break;
    changed = 1;
    while (queriedPostingList[i]->currentValue().docID < k) {
      queriedPostingList[i]->next();
      break;
    }
  }
  return changed;
}

Heap wandTopK(const vector <string> terms, int k) {
  rcout << "Processing " << endl;
  // Memindahkan PostingsList yang diquery ke list of PostingsList
  int n = terms.size();
  vector <PostingsList*> queriedPostingList;
  vector <double> prefixSum(n, 0);
  for (auto cur : terms) {
    queriedPostingList.push_back(posting[cur]);
  }
  int fullyEvalCnt = 0;
  Heap topK;
  for (int i = 0; i < k; i++) {
    topK.push(PostingEntry(-1, 0));
  }
  int curDoc = 0;
  while (true) {
    cout << "----------------" << endl;
    fullSort(queriedPostingList, prefixSum);
    double threshold = topK.top().score;
    ycout << "curDoc: " << curDoc << endl;
    bcout << "threshold: " << threshold << endl;
    rcout << "Cetak Pointer: " << endl;
    gcout << "Heap: " << endl;
    vector <PostingEntry> tmp;
    while (!topK.empty()) {
      bcout << topK.top().docID << " " << topK.top().score << endl;
      tmp.push_back(topK.top());
      topK.pop();
    }
    for (auto cur : tmp) {
      topK.push(cur);
    }
    for (const auto &cur : queriedPostingList) {
      ycout << cur->term << " -> ";
      mcout << cur->upperBoundScore << ": ";
      gcout << "(" << cur->currentValue().docID << ", " << cur->currentValue().score << ")" << endl;
    }
    int pivotIndex = -1;
    for (int i = 0; i < n; i++) {
      if (prefixSum[i] >= threshold) {
        pivotIndex = i;
        break;
      }
    }
    assert(pivotIndex != -1);
    int pivot = queriedPostingList[pivotIndex]->currentValue().docID;
    if (pivot == INT_MAX) break;
    cout << "Ketemu pivot: " << pivotIndex << " " << queriedPostingList[pivotIndex]->term << endl;
    // Shifting is like only shifting the first one into a certain point
    if (queriedPostingList[0]->currentValue().docID == pivot) {
      bcout << "Fully evaluating " << pivot << endl;
      fullyEvalCnt++;
      // Valid stuff, fully evaluate this current docID
      curDoc = pivot;
      double totalScore = 0;
      for (int i = 0; i < n; i++) {
        if (queriedPostingList[i]->currentValue().docID != pivot) break;
        totalScore += queriedPostingList[i]->currentValue().score;
      }
      topK.push({pivot, totalScore});
      topK.pop();
      readjust(queriedPostingList, curDoc + 1);
    } else {
      readjust(queriedPostingList, pivot);
    }
  }
  rcout << "Fully evaluated count: " << fullyEvalCnt << endl;
  while (!topK.empty() && topK.top().score == 0) topK.pop();
  return topK;
}


int main() {
  PostingsList *hujan = new PostingsList("hujan", {{1, 1.5}, {2, 0.4}, {3, 0.6}, {6, 1.0}, {8, 1.5}, {11, 1.6}});
  posting[hujan->term] = hujan;
  PostingsList *turun = new PostingsList("turun", {{1, 0.7}, {3, 1.0}, {6, 1.5}, {8, 1.5}, {10, 0.3}, {12, 1.1}});
  posting[turun->term] = turun;
  PostingsList *deras = new PostingsList("deras", {{1, 1.2}, {6, 1.0}, {7, 0.5}, {10, 0.6}, {11, 1.8}});
  posting[deras->term] = deras;
  auto result = wandTopK({"hujan", "turun", "deras"}, 2);
  while (!result.empty()) {
    ycout << result.top().docID << " " << result.top().score << endl;
    result.pop();
  }
}