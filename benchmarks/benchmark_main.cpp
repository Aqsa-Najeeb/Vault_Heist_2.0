#include "BST.h"
#include "AVL.h"
#include "MaxHeap.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <random>
#include <iomanip>
#include <algorithm>
#include <cmath>

struct RotationCounterLog : public EventLog
{
    int rotations = 0;
    void record(const LogEvent& ev) override
    {
        if (ev.type == EventType::ROTATE)
        {
            rotations++;
        }
    }
    void record(EventType type, const std::string&) override
    {
        if (type == EventType::ROTATE)
        {
            rotations++;
        }
    }
};

struct SortMetrics
{
    long long comparisons = 0;
    long long swaps = 0;
    double timeMs = 0.0;
    size_t auxSpaceBytes = 0;
};

static void insertionSort(int* arr, int n, SortMetrics& metrics)
{
    auto start = std::chrono::high_resolution_clock::now();
    metrics.auxSpaceBytes = sizeof(int);
    for (int i = 1; i < n; ++i)
    {
        int key = arr[i];
        int j = i - 1;
        while (j >= 0)
        {
            metrics.comparisons++;
            if (arr[j] > key)
            {
                arr[j + 1] = arr[j];
                metrics.swaps++;
                j--;
            }
            else
            {
                break;
            }
        }
        arr[j + 1] = key;
    }
    auto end = std::chrono::high_resolution_clock::now();
    metrics.timeMs = std::chrono::duration<double, std::milli>(end - start).count();
}

static void mergeSub(int* arr, int l, int m, int r, int* temp, SortMetrics& metrics)
{
    int i = l;
    int j = m + 1;
    int k = l;

    while (i <= m && j <= r)
    {
        metrics.comparisons++;
        if (arr[i] <= arr[j])
        {
            temp[k++] = arr[i++];
        }
        else
        {
            temp[k++] = arr[j++];
        }
        metrics.swaps++;
    }

    while (i <= m)
    {
        temp[k++] = arr[i++];
        metrics.swaps++;
    }
    while (j <= r)
    {
        temp[k++] = arr[j++];
        metrics.swaps++;
    }

    for (i = l; i <= r; ++i)
    {
        arr[i] = temp[i];
    }
}

static void mergeSortRec(int* arr, int l, int r, int* temp, SortMetrics& metrics)
{
    if (l < r)
    {
        int m = l + (r - l) / 2;
        mergeSortRec(arr, l, m, temp, metrics);
        mergeSortRec(arr, m + 1, r, temp, metrics);
        mergeSub(arr, l, m, r, temp, metrics);
    }
}

static void mergeSort(int* arr, int n, SortMetrics& metrics)
{
    auto start = std::chrono::high_resolution_clock::now();
    metrics.auxSpaceBytes = static_cast<size_t>(n) * sizeof(int);
    int* temp = new int[n];
    mergeSortRec(arr, 0, n - 1, temp, metrics);
    delete[] temp;
    auto end = std::chrono::high_resolution_clock::now();
    metrics.timeMs = std::chrono::duration<double, std::milli>(end - start).count();
}

static int partitionQuick(int* arr, int low, int high, SortMetrics& metrics)
{
    int mid = low + (high - low) / 2;
    int pivot = arr[mid];
    int i = low;
    int j = high;

    while (i <= j)
    {
        while (arr[i] < pivot)
        {
            metrics.comparisons++;
            i++;
        }
        metrics.comparisons++;

        while (arr[j] > pivot)
        {
            metrics.comparisons++;
            j--;
        }
        metrics.comparisons++;

        if (i <= j)
        {
            int t = arr[i];
            arr[i] = arr[j];
            arr[j] = t;
            metrics.swaps++;
            i++;
            j--;
        }
    }
    return i;
}

static void quickSortRec(int* arr, int low, int high, SortMetrics& metrics)
{
    int index = partitionQuick(arr, low, high, metrics);
    if (low < index - 1)
    {
        quickSortRec(arr, low, index - 1, metrics);
    }
    if (index < high)
    {
        quickSortRec(arr, index, high, metrics);
    }
}

static void quickSort(int* arr, int n, SortMetrics& metrics)
{
    auto start = std::chrono::high_resolution_clock::now();
    metrics.auxSpaceBytes = static_cast<size_t>(std::log2(n > 0 ? n : 1)) * sizeof(int*);
    if (n > 1)
    {
        quickSortRec(arr, 0, n - 1, metrics);
    }
    auto end = std::chrono::high_resolution_clock::now();
    metrics.timeMs = std::chrono::duration<double, std::milli>(end - start).count();
}

static void heapSortInstrumented(int* arr, int n, SortMetrics& metrics)
{
    auto start = std::chrono::high_resolution_clock::now();
    metrics.auxSpaceBytes = sizeof(int);

    auto siftDownInst = [&](int idx, int sz) {
        int curr = idx;
        while (true)
        {
            int largest = curr;
            int l = 2 * curr + 1;
            int r = 2 * curr + 2;

            if (l < sz)
            {
                metrics.comparisons++;
                if (arr[l] > arr[largest])
                {
                    largest = l;
                }
            }
            if (r < sz)
            {
                metrics.comparisons++;
                if (arr[r] > arr[largest])
                {
                    largest = r;
                }
            }

            if (largest != curr)
            {
                int t = arr[curr];
                arr[curr] = arr[largest];
                arr[largest] = t;
                metrics.swaps++;
                curr = largest;
            }
            else
            {
                break;
            }
        }
    };

    for (int i = (n - 2) / 2; i >= 0; --i)
    {
        siftDownInst(i, n);
    }

    for (int i = n - 1; i > 0; --i)
    {
        int t = arr[0];
        arr[0] = arr[i];
        arr[i] = t;
        metrics.swaps++;
        siftDownInst(0, i);
    }

    auto end = std::chrono::high_resolution_clock::now();
    metrics.timeMs = std::chrono::duration<double, std::milli>(end - start).count();
}

int main()
{
    std::ofstream csv("C:\\Users\\jensc\\repos\\Vault2\\Vault_Heist_2.0\\benchmarks\\results.csv");
    if (!csv.is_open())
    {
        csv.open("C:\\Users\\jensc\\repos\\Vault2\\Vault_Heist_2.0\\benchmarks\\results.csv");
    }
    std::ofstream txt("C:\\Users\\jensc\\repos\\Vault2\\Vault_Heist_2.0\\benchmarks\\results.txt");
    if (!txt.is_open())
    {
        txt.open("C:\\Users\\jensc\\repos\\Vault2\\Vault_Heist_2.0\\benchmarks\\results.txt");
    }

    auto logBoth = [&](const std::string& line) {
        std::cout << line << std::endl;
        txt << line << std::endl;
    };

    csv << "Experiment,N,InputType,Algorithm,Metric1_Name,Metric1_Val,Metric2_Name,Metric2_Val,Metric3_Name,Metric3_Val\n";

    logBoth("================================================================================");
    logBoth("                VAULT HEIST 2.0 - COMPREHENSIVE BENCHMARK REPORT                ");
    logBoth("================================================================================\n");

    logBoth("--- EXPERIMENT 1: BST vs AVL Tree Height ---");
    logBoth(std::string("N\t\tInput\t\tBST Height\tAVL Height\tTheoretical Optimal (log2 N)"));

    int testSizes[] = { 100, 1000, 10000 };
    for (int n : testSizes)
    {
        std::vector<int> sortedVals(n);
        for (int i = 0; i < n; ++i) sortedVals[i] = i + 1;

        std::vector<int> randVals = sortedVals;
        std::mt19937 g(1337 + n);
        std::shuffle(randVals.begin(), randVals.end(), g);

        BST bstSorted;
        AVLTree avlSorted;
        for (int v : sortedVals)
        {
            bstSorted.insert(v, nullptr);
            avlSorted.insert(v, nullptr);
        }

        int hBstSorted = bstSorted.height();
        int hAvlSorted = avlSorted.treeHeight();

        logBoth(std::to_string(n) + "\t\tSorted\t\t" + std::to_string(hBstSorted) + "\t\t" + std::to_string(hAvlSorted) + "\t\t" + std::to_string(static_cast<int>(std::ceil(std::log2(n + 1)))));
        csv << "Height," << n << ",Sorted,BST,Height," << hBstSorted << ",,,,\n";
        csv << "Height," << n << ",Sorted,AVL,Height," << hAvlSorted << ",,,,\n";

        BST bstRand;
        AVLTree avlRand;
        for (int v : randVals)
        {
            bstRand.insert(v, nullptr);
            avlRand.insert(v, nullptr);
        }

        int hBstRand = bstRand.height();
        int hAvlRand = avlRand.treeHeight();

        logBoth(std::to_string(n) + "\t\tRandom\t\t" + std::to_string(hBstRand) + "\t\t" + std::to_string(hAvlRand) + "\t\t" + std::to_string(static_cast<int>(std::ceil(std::log2(n + 1)))));
        csv << "Height," << n << ",Random,BST,Height," << hBstRand << ",,,,\n";
        csv << "Height," << n << ",Random,AVL,Height," << hAvlRand << ",,,,\n";
    }

    logBoth("\n--- EXPERIMENT 2: Search Cost (10,000 Random Searches) ---");
    logBoth("N\t\tData Structure\tAvg Comparisons\tTotal Time (ms)\tAvg Time/Search (ns)");

    for (int n : testSizes)
    {
        std::vector<int> randVals(n);
        for (int i = 0; i < n; ++i) randVals[i] = i * 2;
        std::mt19937 g(42 + n);
        std::shuffle(randVals.begin(), randVals.end(), g);

        BST bst;
        AVLTree avl;
        for (int v : randVals)
        {
            bst.insert(v, nullptr);
            avl.insert(v, nullptr);
        }

        std::vector<int> queryKeys(10000);
        for (int i = 0; i < 10000; ++i) queryKeys[i] = g() % (2 * n + 1);

        long long bstComps = 0;
        auto bstStart = std::chrono::high_resolution_clock::now();
        for (int q : queryKeys)
        {
            TreeNode* curr = bst.getRoot();
            while (curr)
            {
                bstComps++;
                if (curr->value == q) break;
                if (q < curr->value) curr = curr->left;
                else curr = curr->right;
            }
        }
        auto bstEnd = std::chrono::high_resolution_clock::now();
        double bstTime = std::chrono::duration<double, std::milli>(bstEnd - bstStart).count();

        long long avlComps = 0;
        auto avlStart = std::chrono::high_resolution_clock::now();
        for (int q : queryKeys)
        {
            AVLNode* curr = avl.getRoot();
            while (curr)
            {
                avlComps++;
                if (curr->value == q) break;
                if (q < curr->value) curr = curr->left;
                else curr = curr->right;
            }
        }
        auto avlEnd = std::chrono::high_resolution_clock::now();
        double avlTime = std::chrono::duration<double, std::milli>(avlEnd - avlStart).count();

        double bstAvgC = static_cast<double>(bstComps) / 10000.0;
        double avlAvgC = static_cast<double>(avlComps) / 10000.0;
        double bstNs = (bstTime * 1e6) / 10000.0;
        double avlNs = (avlTime * 1e6) / 10000.0;

        logBoth(std::to_string(n) + "\t\tBST\t\t" + std::to_string(bstAvgC) + "\t\t" + std::to_string(bstTime) + "\t\t" + std::to_string(bstNs));
        logBoth(std::to_string(n) + "\t\tAVL\t\t" + std::to_string(avlAvgC) + "\t\t" + std::to_string(avlTime) + "\t\t" + std::to_string(avlNs));

        csv << "Search," << n << ",Random,BST,AvgComps," << bstAvgC << ",TotalTimeMs," << bstTime << ",AvgTimeNs," << bstNs << "\n";
        csv << "Search," << n << ",Random,AVL,AvgComps," << avlAvgC << ",TotalTimeMs," << avlTime << ",AvgTimeNs," << avlNs << "\n";
    }

    logBoth("\n--- EXPERIMENT 3: AVL Rotations per Operation ---");
    logBoth("N\t\tInput\t\tAvg Rot/Insert\tAvg Rot/Delete\tMax Rot/Delete");

    for (int n : testSizes)
    {
        for (int type = 0; type < 2; ++type)
        {
            std::string inputName = (type == 0 ? "Sorted" : "Random");
            std::vector<int> vals(n);
            for (int i = 0; i < n; ++i) vals[i] = i + 1;
            if (type == 1)
            {
                std::mt19937 g(777 + n);
                std::shuffle(vals.begin(), vals.end(), g);
            }

            AVLTree avl;
            RotationCounterLog rotLog;
            rotLog.setEnabled(true);

            long long insertRotations = 0;
            for (int v : vals)
            {
                rotLog.rotations = 0;
                avl.insert(v, &rotLog);
                insertRotations += rotLog.rotations;
            }

            long long deleteRotations = 0;
            int maxDeleteRot = 0;
            for (int v : vals)
            {
                rotLog.rotations = 0;
                avl.deleteKey(v, &rotLog);
                deleteRotations += rotLog.rotations;
                if (rotLog.rotations > maxDeleteRot)
                {
                    maxDeleteRot = rotLog.rotations;
                }
            }

            double avgIns = static_cast<double>(insertRotations) / static_cast<double>(n);
            double avgDel = static_cast<double>(deleteRotations) / static_cast<double>(n);

            logBoth(std::to_string(n) + "\t\t" + inputName + "\t\t" + std::to_string(avgIns) + "\t" + std::to_string(avgDel) + "\t" + std::to_string(maxDeleteRot));
            csv << "Rotations," << n << "," << inputName << ",AVL,AvgRotInsert," << avgIns << ",AvgRotDelete," << avgDel << ",MaxRotDelete," << maxDeleteRot << "\n";
        }
    }

    logBoth("\n--- EXPERIMENT 4: Order Statistics (1,000 Queries: AVL kth vs BST Inorder Scan) ---");
    logBoth("N\t\tAVL kth (ms)\tBST Inorder (ms)\tSpeedup Factor");

    for (int n : testSizes)
    {
        std::vector<int> vals(n);
        for (int i = 0; i < n; ++i) vals[i] = (i + 1) * 3;
        std::mt19937 g(999 + n);
        std::shuffle(vals.begin(), vals.end(), g);

        BST bst;
        AVLTree avl;
        for (int v : vals)
        {
            bst.insert(v, nullptr);
            avl.insert(v, nullptr);
        }

        std::vector<int> kQueries(1000);
        for (int i = 0; i < 1000; ++i) kQueries[i] = (g() % n) + 1;

        auto avlKthStart = std::chrono::high_resolution_clock::now();
        volatile int dummyAVL = 0;
        for (int k : kQueries)
        {
            dummyAVL += avl.kth(k);
        }
        auto avlKthEnd = std::chrono::high_resolution_clock::now();
        double avlKthMs = std::chrono::duration<double, std::milli>(avlKthEnd - avlKthStart).count();

        auto bstKthStart = std::chrono::high_resolution_clock::now();
        volatile int dummyBST = 0;
        for (int k : kQueries)
        {
            dummyBST += bst.kthSmallest(k);
        }
        auto bstKthEnd = std::chrono::high_resolution_clock::now();
        double bstKthMs = std::chrono::duration<double, std::milli>(bstKthEnd - bstKthStart).count();

        double speedup = bstKthMs / (avlKthMs > 1e-6 ? avlKthMs : 1e-6);
        logBoth(std::to_string(n) + "\t\t" + std::to_string(avlKthMs) + "\t\t" + std::to_string(bstKthMs) + "\t\t" + std::to_string(speedup) + "x");
        csv << "OrderStatistics," << n << ",Random,Comparison,AVLTimeMs," << avlKthMs << ",BSTTimeMs," << bstKthMs << ",Speedup," << speedup << "\n";
    }

    logBoth("\n--- EXPERIMENT 5: Bottom-Up Heap Construction Time (Confirming O(n)) ---");
    logBoth("N\t\tInput\t\tBuild Time (ms)\tTime / N (ns)");

    for (int n : testSizes)
    {
        for (int type = 0; type < 2; ++type)
        {
            std::string inputName = (type == 0 ? "Ordered" : "Random");
            std::vector<int> data(n);
            for (int i = 0; i < n; ++i) data[i] = i + 1;
            if (type == 1)
            {
                std::mt19937 g(101 + n);
                std::shuffle(data.begin(), data.end(), g);
            }

            const int trials = 50;
            auto hStart = std::chrono::high_resolution_clock::now();
            for (int t = 0; t < trials; ++t)
            {
                MaxHeap h;
                h.buildMaxHeap(data.data(), n, nullptr);
            }
            auto hEnd = std::chrono::high_resolution_clock::now();
            double totalMs = std::chrono::duration<double, std::milli>(hEnd - hStart).count();
            double avgMs = totalMs / static_cast<double>(trials);
            double nsPerN = (avgMs * 1e6) / static_cast<double>(n);

            logBoth(std::to_string(n) + "\t\t" + inputName + "\t\t" + std::to_string(avgMs) + "\t\t" + std::to_string(nsPerN));
            csv << "HeapBuild," << n << "," << inputName << ",MaxHeap,BuildTimeMs," << avgMs << ",NsPerN," << nsPerN << ",,\n";
        }
    }

    logBoth("\n--- EXPERIMENT 6: Heap Sort Across Input Distributions (N = 10,000) ---");
    logBoth("Input Distribution\tTime (ms)\tComparisons\tSwaps");

    int nHeap = 10000;
    std::string distributions[] = { "Random", "Sorted", "Reverse-Sorted", "Duplicate-Heavy" };
    for (const auto& dist : distributions)
    {
        std::vector<int> data(nHeap);
        if (dist == "Random")
        {
            std::mt19937 g(54321);
            for (int i = 0; i < nHeap; ++i) data[i] = g() % 100000;
        }
        else if (dist == "Sorted")
        {
            for (int i = 0; i < nHeap; ++i) data[i] = i;
        }
        else if (dist == "Reverse-Sorted")
        {
            for (int i = 0; i < nHeap; ++i) data[i] = nHeap - i;
        }
        else
        {
            for (int i = 0; i < nHeap; ++i) data[i] = i % 5;
        }

        SortMetrics m;
        heapSortInstrumented(data.data(), nHeap, m);
        logBoth(dist + "\t\t" + std::to_string(m.timeMs) + "\t\t" + std::to_string(m.comparisons) + "\t\t" + std::to_string(m.swaps));
        csv << "HeapSortDistribution," << nHeap << "," << dist << ",HeapSort,TimeMs," << m.timeMs << ",Comparisons," << m.comparisons << ",Swaps," << m.swaps << "\n";
    }

    logBoth("\n--- EXPERIMENT 7: Sorting Algorithms Comparison (Heap Sort vs Insertion, Merge, Quick) ---");
    logBoth("N\tAlgorithm\tRuntime (ms)\tComparisons\tSwaps/Moves\tAuxiliary Space");

    int sortSizes[] = { 100, 1000, 5000, 10000 };
    for (int n : sortSizes)
    {
        std::vector<int> orig(n);
        std::mt19937 g(888 + n);
        for (int i = 0; i < n; ++i) orig[i] = g() % 100000;

        std::vector<int> dHeap = orig;
        SortMetrics mHeap;
        heapSortInstrumented(dHeap.data(), n, mHeap);
        logBoth(std::to_string(n) + "\tHeapSort\t" + std::to_string(mHeap.timeMs) + "\t\t" + std::to_string(mHeap.comparisons) + "\t\t" + std::to_string(mHeap.swaps) + "\t\tO(1) [" + std::to_string(mHeap.auxSpaceBytes) + " B]");
        csv << "SortComparison," << n << ",Random,HeapSort,TimeMs," << mHeap.timeMs << ",Comparisons," << mHeap.comparisons << ",Swaps," << mHeap.swaps << "\n";

        std::vector<int> dMerge = orig;
        SortMetrics mMerge;
        mergeSort(dMerge.data(), n, mMerge);
        logBoth(std::to_string(n) + "\tMergeSort\t" + std::to_string(mMerge.timeMs) + "\t\t" + std::to_string(mMerge.comparisons) + "\t\t" + std::to_string(mMerge.swaps) + "\t\tO(n) [" + std::to_string(mMerge.auxSpaceBytes) + " B]");
        csv << "SortComparison," << n << ",Random,MergeSort,TimeMs," << mMerge.timeMs << ",Comparisons," << mMerge.comparisons << ",Swaps," << mMerge.swaps << "\n";

        std::vector<int> dQuick = orig;
        SortMetrics mQuick;
        quickSort(dQuick.data(), n, mQuick);
        logBoth(std::to_string(n) + "\tQuickSort\t" + std::to_string(mQuick.timeMs) + "\t\t" + std::to_string(mQuick.comparisons) + "\t\t" + std::to_string(mQuick.swaps) + "\t\tO(log n) [" + std::to_string(mQuick.auxSpaceBytes) + " B]");
        csv << "SortComparison," << n << ",Random,QuickSort,TimeMs," << mQuick.timeMs << ",Comparisons," << mQuick.comparisons << ",Swaps," << mQuick.swaps << "\n";

        if (n <= 5000)
        {
            std::vector<int> dIns = orig;
            SortMetrics mIns;
            insertionSort(dIns.data(), n, mIns);
            logBoth(std::to_string(n) + "\tInsertionSort\t" + std::to_string(mIns.timeMs) + "\t\t" + std::to_string(mIns.comparisons) + "\t\t" + std::to_string(mIns.swaps) + "\t\tO(1) [" + std::to_string(mIns.auxSpaceBytes) + " B]");
            csv << "SortComparison," << n << ",Random,InsertionSort,TimeMs," << mIns.timeMs << ",Comparisons," << mIns.comparisons << ",Swaps," << mIns.swaps << "\n";
        }
        else
        {
            logBoth(std::to_string(n) + "\tInsertionSort\t[SKIPPED O(N^2)]\t---\t\t---\t\tO(1)");
        }
    }

    logBoth("\n================================================================================");
    logBoth("                          BENCHMARKS COMPLETE                                   ");
    logBoth("================================================================================");

    csv.close();
    txt.close();
    return 0;
}
