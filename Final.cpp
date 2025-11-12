#include <iostream>
#include <stack>
#include <vector>
#include <chrono>
#include <string>
#include <conio.h> // for getch() on Windows

using namespace std;
using namespace std::chrono;

struct Action {
    string textSnapshot;
    long long timestamp;
};

stack<Action> undoStack;       // Smart undo/redo
stack<Action> redoStack;       // Smart redo
vector<Action> historyStack;   // Permanent snapshot storage (never modified)

string currentText = "";
long long lastKeypressTime = 0;
const long long GROUP_THRESHOLD = 900; // ms grouping

// Get current timestamp in ms
long long nowMs() {
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

// Text Editor
void typeText() {
    redoStack = stack<Action>(); // clear redo history

    cout << "Start typing (Press ENTER to finish): ";

    char c;
    while (true) {
        c = _getch();  

        if (c == 13) break; // stop on ENTER key

        cout << c;

        long long currentTime = nowMs();

        // First char or new typing group ( > threshold)
        if (undoStack.empty() || currentTime - lastKeypressTime > GROUP_THRESHOLD) {
            undoStack.push({currentText + c, currentTime});
        }
        else {
            // Update latest group
            undoStack.top().textSnapshot += c;
        }

        // Update current text
        currentText = undoStack.top().textSnapshot;
        lastKeypressTime = currentTime;

        // Push *every snapshot* permanently to history
        historyStack.push_back({currentText, currentTime});
    }

    cout << endl;
}

// Undo
void undo() {
    if (undoStack.empty()) return;

    redoStack.push(undoStack.top());
    undoStack.pop();

    currentText = undoStack.empty() ? "" : undoStack.top().textSnapshot;
}

// Redo
void redo() {
    if (redoStack.empty()) return;

    undoStack.push(redoStack.top());
    currentText = redoStack.top().textSnapshot;
    redoStack.pop();
}

// History Display
void showHistory() {
    cout << "\n--- Permanent Snapshot History (Never Deleted) ---\n";
    for (int i = 0; i < historyStack.size(); i++)
        cout << i << ": \"" << historyStack[i].textSnapshot << "\"\n";
}

// Selective Undo
// Revert to ANY old snapshot, push to undoStack, but do NOT modify historyStack
void selectiveUndo() {
    if (historyStack.empty()) {
        cout << "\nNo snapshots stored yet.\n";
        return;
    }

    showHistory();

    cout << "\nSelect snapshot index to revert: ";
    int idx;
    cin >> idx;

    if (idx < 0 || idx >= historyStack.size()) {
        cout << "Invalid snapshot index.\n";
        return;
    }

    // Update text to that snapshot
    currentText = historyStack[idx].textSnapshot;

    // Push this state to undo stack so normal undo can continue from here
    undoStack.push({currentText, nowMs()});

    cout << "\n Selective undo applied successfully!\n";
}

// Main Loop
int main() {
    int choice;

    while (true) {
        cout << "\nCurrent text: \"" << currentText << "\"\n";
        cout << "1.Type  2.Undo  3.Redo  4.Selective Undo  5.Show History  0.Exit\n";
        cin >> choice;

        switch (choice) {
            case 1: typeText(); break;
            case 2: undo(); break;
            case 3: redo(); break;
            case 4: selectiveUndo(); break;
            case 5: showHistory(); break;
            case 0: return 0;
        }
    }
}
