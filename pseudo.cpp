Function next(θ):
    repeat:
        // O(Q log Q)
        quickSort(terms, posting)
        // O(Q)
        computePrefixSum(terms, posting)
        // O(Q)
        pterm := getPivot(terms, posting, threshold)
        if(pivot == lastID) return (NoMoreDocs)
        // kasus pivot <= curDoc tidak mungkin terjadi
        if(pivot == posting[0].DID):
            //Assign jawaban
            curDoc := pivot
            // O(Q)
            evaluateFull(pivot)
            // O(Q)
            aterm := (posting if posting.iterator == pivot)
            // Amortized O(Σ len(posting))
            // Majuin semua yang sama dengan pivot ke >= curDoc + 1
            posting[aterm] = aterm.iterator.next(curDoc + 1)
        else:
            // Massnya tidak cukup
            // Disini mungkin bisa dioptimisasi lebih kencang
            aterm := (posting if posting.iterator < pivot)
            // Amortized O(Σ len(posting))
            // Majuin semua yang sama dengan pivot ke >= pivot
            posting[aterm] = aterm.iterator.next(pivot)
    end repeat
