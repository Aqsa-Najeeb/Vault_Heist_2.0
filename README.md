# Vault Heist 2.0: The Living Vault

## 1. Project Description
Vault Heist 2.0 is a puzzle-strategy game where the player infiltrates a living vault whose chambers form a tree data structure. Every room entered or key compared charges a physical security **Alarm Meter**, creating a tangible connection between tree height, balance, and escape success.

The game is organized into four campaign acts (20 levels total) plus Endless and Sandbox modes:
- **Act 1: The Blueprint Wing (General Binary Tree):** Traversal predictions (preorder, inorder, postorder, level order), tree reconstruction from blueprint traversals, leaf-to-root height computation, lowest common ancestor (LCA), and tree inversion (mirroring).
- **Act 2: The Sorted Wing (Binary Search Tree):** Navigation comparisons, ordered insertion, node deletion cases (leaf, single-child bypass, two-child successor swap), range query subtree pruning, and degenerated chain survival.
- **Act 3: The Architect's Wing (AVL Tree):** Real-time balance factor monitoring, manual single/double rotations (LL, RR, LR, RL), auto-rebalancing prediction, and augmented order statistics ($O(\log n)$ $k$-th smallest and rank retrieval using `subtreeSize`).
- **Act 4: The Heap Fortress (Max Heap & Heap Sort):** Complete binary tree indexing, bottom-up $O(n)$ Max Heap construction, root extraction, sift-down heap restoration, and full step-by-step Heap Sort with synchronized tree and array views.

All core data structures (`BinaryTree`, `BST`, `AVLTree`, `MaxHeap`, `EventLog`, `Validator`) are built from scratch without standard library container implementations.

---

## 2. Requirements & Environment
- **Operating System:** Windows 10/11, Linux, or macOS
- **Compiler:** C++17 compliant compiler (`g++` >= 9.0, `clang++` >= 10.0, or MSVC 2019/2022)
- **Build System:** CMake >= 3.20
- **Libraries:**
  - SFML 3.1.0 (automatically resolved via CMake FetchContent)
  - nlohmann_json 3.11.3 (automatically resolved via CMake FetchContent)
- **Optional Tools:** Python 3.9+ with `matplotlib` for benchmark plotting and PDF report generation.

---

## 3. Build & Run Instructions

### 3.1 Building the Game
From the root of the repository:
```bash
cmake -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build
```
*(On Visual Studio, open the repository folder directly or generate using `cmake -B build` without generator flags).*

### 3.2 Running the Game
```bash
./build/bin/main.exe
```

---

## 4. Building and Running Tests & Benchmarks

### 4.1 Running the Unit Test Suite (Phase 1 & 2)
The test suite is model-only (headless, zero GUI dependencies) and exercises all 11 specification areas with 36 automated test cases:
```bash
cd tests
g++ -std=c++17 -Wall -Wextra -I../src -I. -o run_tests.exe ../src/BinaryTree.cpp ../src/BST.cpp ../src/AVL.cpp ../src/MaxHeap.cpp ../src/Validator.cpp ../src/EventLog.cpp test_main.cpp test_traversals.cpp test_reconstruction.cpp test_bst.cpp test_avl.cpp test_order_statistics.cpp test_duplicates.cpp test_max_heap.cpp test_heap_sort.cpp test_event_undo.cpp test_save_load.cpp test_event_disable.cpp
./run_tests.exe
```
Or using CMake:
```bash
cd tests
cmake -B build -G "MinGW Makefiles"
cmake --build build
./build/run_tests.exe
```

### 4.2 Running the Benchmarks (Phase 5)
Compiles with `-O2` and runs all 7 empirical experiments (height degeneration, 10,000 random searches, rotation frequencies, order statistics speedup, heap construction linearity, and sorting algorithms comparison):
```bash
cd benchmarks
g++ -O2 -std=c++17 -Wall -Wextra -I../src -o benchmark.exe benchmark_main.cpp ../src/BST.cpp ../src/AVL.cpp ../src/MaxHeap.cpp ../src/Validator.cpp ../src/EventLog.cpp ../src/BinaryTree.cpp
./benchmark.exe
```
Benchmark outputs are written to `benchmarks/results.csv` and `benchmarks/results.txt`.

## 5. Folder Structure
```
Vault_Heist_2.0/
├── CMakeLists.txt              # Root build configuration linking SFML and model library
├── README.md                   # Complete documentation and user guide
├── src/                        # Core source code
│   ├── BinaryTree.h / .cpp     # General binary tree, traversals, reconstruction, mirror
│   ├── BST.h / .cpp            # Binary Search Tree with 3 deletion cases & range pruning
│   ├── AVL.h / .cpp            # Augmented AVL tree with LL/RR/LR/RL rotations & kth/rank
│   ├── MaxHeap.h / .cpp        # Array-based Max Heap & in-place Heap Sort
│   ├── Validator.h / .cpp      # Structural and mathematical invariant verification
│   ├── EventLog.h / .cpp       # Decoupled event logging and dispatch model
│   ├── GameModes.h / .cpp      # Endless mode, Sandbox editors, audio and settings
│   ├── Level.cpp / Map.cpp     # Campaign level loop, HUD, tree canvas layout & drawing
│   ├── Menu.cpp / User.cpp     # Main navigation menus and user progress persistence
│   ├── assets/                 # Fonts, sound effects, background music, level art
│   └── data/                   # levels.json and vault_layout.json
├── benchmarks/                 # Standalone benchmarking suite
│   ├── benchmark_main.cpp      # Section 10 empirical benchmarking program
│   ├── results.csv             # Structured CSV dataset from benchmark run
│   └── results.txt             # Formatted benchmark summary tables
└── data/                       # Save state files (progress.sav, settings.json)
```

---

## 6. Game Controls

### Mouse Controls
- **Left Click:** Select room, interact with UI buttons, choose rotation pivots, or swap array elements.
- **Hover:** Displays inspection tooltip with room key, subtree height, balance factor, node size, and tree depth.
- **Left Click + Drag:** 
  - Drag blueprint room tokens into slots (reconstruction levels).
  - Drag subtrees onto rotation pivots for manual AVL rebalancing.
- **Mouse Wheel:** Smooth canvas zoom in / zoom out.
- **Right Click / Middle Click Drag:** Pan viewport across large trees or arrays.

### Keyboard Shortcuts
- **Space:** Play / pause event animation playback.
- **Left Arrow:** Step backward one event in history.
- **Right Arrow:** Step forward one event in history.
- **Z:** Undo last player action (restores previous tree/heap snapshot).
- **H:** Reveal strategic hint (-1 star penalty cap).
- **Esc:** Open pause menu / return to campaign map.



## 7. Guide to Adding New Levels via JSON Data Files

Vault Heist 2.0 separates game data from engine logic. Level definitions, generation parameters, objectives, coordinate layouts, and UI button configurations are driven entirely by external JSON files. Adding new custom levels requires zero C++ recompilation when using existing task mechanics.

---

### 7.1 Architecture Overview

Level loading and gameplay execution flow through two configuration files:
1. `src/data/levels.json` (fallback: `data/levels.json`): Governs level metadata, Act category, task objective, deterministic PRNG seed, and node generator rules.
2. `src/data/vault_layout.json` (fallback: `data/vault_layout.json`): Governs visual coordinate layouts in the 1920x1080 design space, including canvas bounds, button positions/labels, input boxes, and array strip cells.

Campaign progression and unlock sequence are managed in `src/User.cpp` via player save data in `data/progress.sav` (or `src/data/progress.sav`).

---

### 7.2 Step-by-Step Level Creation Guide

#### Step 1: Register Level Logic in `src/data/levels.json`

Locate the `"levels"` array in `src/data/levels.json` and append your new level object.

```json
{
  "id": "2.6",
  "act": 2,
  "seed": 2601,
  "image": "level_2_1.png",
  "task": "range",
  "generator": {
    "nodes": [8, 12],
    "values": "distinct",
    "range": [25, 75]
  }
}
```

#### Field Specifications:
- **`id` (string):** The unique identifier. Both dot-notation (`"2.6"`) and underscore-notation (`"2_6"`) are automatically recognized and normalized by `Level::run`.
- **`act` (integer):** The Act classification:
  - `1`: General Binary Trees (Traversals, Blueprint Reconstruction, Height/Diameter, LCA, Mirroring).
  - `2`: Binary Search Trees (Search, Insertion, Deletion, Range Queries, kth-Smallest).
  - `3`: AVL Trees (Self-Balancing Invariants, LL/RR/LR/RL Rotations, Height/BF verification).
  - `4`: Max Heaps & Heap Sort (Heap Property Verification, Sift-Down, Extract-Max, Heap Sort).
- **`seed` (integer):** A deterministic random seed ensuring consistent, reproducible tree generation for each player attempt.
- **`image` (string):** The background texture file located inside `src/assets/images/` (e.g. `"level_2_1.png"`, `"Sandbox_BST.png"`).
- **`task` (string):** The game objective and validator logic to execute. Supported tasks include:
  - `"traversal"`: Execute preorder, inorder, postorder, or level-order visits.
  - `"reconstruct"`: Reconstruct the binary tree blueprint from preorder+inorder or postorder+inorder sequences.
  - `"height_diameter"`: Compute height and maximum path diameter across tree nodes.
  - `"lca"`: Identify the Lowest Common Ancestor between target vault nodes.
  - `"mirror"`: Invert/mirror the vault layout and verify symmetric properties.
  - `"search"`: Locate target security keys using binary search comparisons.
  - `"insert"`: Insert keys maintaining BST ordering without exceeding alarm thresholds.
  - `"delete"`: Safely remove keys (leaf, single child, two children with predecessor/successor).
  - `"range"`: Identify and extract all keys falling within an inclusive interval `[low, high]`.
  - `"kth_smallest"`: Query the $k$-th smallest element using order-statistics metadata.
  - `"avl_balance"`: Restore AVL balance factor $|BF| \le 1$ using rotations.
  - `"rotations"`: Specific rotation training (Single Left/Right, Double Left-Right/Right-Left).
  - `"heap_verify"`: Verify the Max Heap parent-child invariant $A[i] \ge \max(A[2i+1], A[2i+2])$.
  - `"sift_down"`: Restore the heap invariant from a disturbed root using iterative sift-down swaps.
  - `"heap_extract"`: Extract the maximum root key, swap with the last leaf, shrink active region, and sift down.
  - `"heap_sort"`: Perform in-place heap sort, observing the boundary between the active heap and sorted region.
- **`generator` (object):** Configuration for procedural tree generation:
  - `"nodes"`: Array `[min, max]` defining the node count range.
  - `"values"`: `"distinct"` for unique node values, or `"any"` if duplicates are permitted.
  - `"modes"` / `"sources"`: Optional mode constraints (e.g., `["preorder", "inorder"]`).

---

#### Step 2: Define UI Coordinates in `src/data/vault_layout.json`

Under the `"levels"` object in `src/data/vault_layout.json`, add an entry keyed by the level identifier (e.g. `"2.6"` or `"2_6"`):

```json
"2.6": {
  "image": "level_2_1.png",
  "tree_area": { "x": 398, "y": 265, "w": 1100, "h": 648 },
  "buttons": [
    {
      "id": "query_range",
      "label": "Execute Range [25, 75]",
      "x": 54,
      "y": 384,
      "w": 251,
      "h": 77
    },
    {
      "id": "clear_selection",
      "label": "Reset Query",
      "x": 54,
      "y": 480,
      "w": 251,
      "h": 77
    }
  ]
}
```

For Act 4 (Max Heap) levels, also specify the array visualization strip:

```json
"4.6": {
  "image": "level_4_1.png",
  "tree_area": { "x": 398, "y": 220, "w": 1100, "h": 540 },
  "array_strip": {
    "cells": [
      { "index": 0, "x": 520, "y": 820, "w": 128, "h": 64 },
      { "index": 1, "x": 608, "y": 820, "w": 88,  "h": 64 },
      { "index": 2, "x": 696, "y": 820, "w": 88,  "h": 64 },
      { "index": 3, "x": 784, "y": 820, "w": 88,  "h": 64 },
      { "index": 4, "x": 872, "y": 820, "w": 88,  "h": 64 },
      { "index": 5, "x": 960, "y": 820, "w": 88,  "h": 64 },
      { "index": 6, "x": 1048, "y": 820, "w": 88, "h": 64 },
      { "index": 7, "x": 1136, "y": 820, "w": 88, "h": 64 },
      { "index": 8, "x": 1224, "y": 820, "w": 88, "h": 64 },
      { "index": 9, "x": 1312, "y": 820, "w": 128, "h": 64 }
    ]
  },
  "buttons": [
    { "id": "sift_down", "label": "Sift Down Step", "x": 54, "y": 384, "w": 251, "h": 77 }
  ]
}
```

If an interactive button accepts numerical player input (e.g., search key or insertion value), include the `"input"` child object:
```json
{
  "id": "search_key",
  "label": "Probe Node",
  "x": 54, "y": 384, "w": 251, "h": 77,
  "input": { "x": 315, "y": 394, "w": 65, "h": 55 }
}
```

---

#### Step 3: Register Campaign Map and Progression (Optional for Extra Campaign Levels)

The standard campaign contains 20 levels (5 per Act). If adding a 6th level to an Act:

1. **Map Coordinates (`src/Map.cpp`):**
   Add the button coordinates to the corresponding Act pin array (`act1X`, `act1Y`, `act2X`, etc.) to render the level pin on the campaign map:
   ```cpp
   const float act2X[6] = { 650.f, 790.f, 950.f, 1100.f, 1260.f, 1420.f };
   const float act2Y[6] = { 460.f, 440.f, 480.f,  450.f,  490.f,  470.f };
   ```

2. **Progression Registration (`src/User.cpp`):**
   Update the ID table in `levelIdAt(int index)` and adjust the array bounds (`ids[20]` $\rightarrow$ `ids[24]`):
   ```cpp
   const char* levelIdAt(int index)
   {
       static const char* ids[24] =
       {
           "1_1", "1_2", "1_3", "1_4", "1_5", "1_6",
           "2_1", "2_2", "2_3", "2_4", "2_5", "2_6",
           "3_1", "3_2", "3_3", "3_4", "3_5", "3_6",
           "4_1", "4_2", "4_3", "4_4", "4_5", "4_6"
       };
       return ids[index];
   }
   ```
   Levels automatically unlock sequentially: clearing level $i$ with $\ge 1$ star unlocks level $i + 1$.

---

#### Step 4: Verification Checklist

1. **JSON Syntax:** Validate `src/data/levels.json` and `src/data/vault_layout.json` with a JSON linter to avoid parsing syntax errors.
2. **Asset Presence:** Ensure the background image referenced in `"image"` exists inside `src/assets/images/`.
3. **Deterministic Testing:** Run the test suite (`cd tests && ./run_tests.exe`) to confirm that all data structure validators pass against the procedural generators.
