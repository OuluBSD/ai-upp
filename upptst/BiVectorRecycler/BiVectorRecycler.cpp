#include <Core/Core.h>

using namespace Upp;

struct TestType {
	int a;
	int payload[64]; // Make it a bit heavier
	TestType() { a = 0; }
};

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT | LOG_FILE);

	// Basic functionality test
	{
		BiVectorRecycler<TestType, true> q;

		TestType* t1 = q.AddTail();
		t1->a = 1;
		TestType* t2 = q.AddTail();
		t2->a = 2;

		ASSERT(q.GetCount() == 2);
		ASSERT(q[0].a == 1);
		ASSERT(q[1].a == 2);

		BiVectorRecycler<TestType, true> q2;
		q2 = pick(q);

		ASSERT(q.GetCount() == 0);
		ASSERT(q2.GetCount() == 2);
		ASSERT(q2[0].a == 1);
		ASSERT(q2[1].a == 2);

		TestType* ptr_addr = &q2[0];
		q2.DropHead();
		ASSERT(q2.GetCount() == 1);
		ASSERT(q2[0].a == 2);

		// Test reuse
		TestType* t3 = q2.AddTail();
		t3->a = 3;

		// Ideally, t3 should be the recycled ptr_addr
		bool recycled = (t3 == ptr_addr);
		Cout() << "Pointer recycled: " << (recycled ? "Yes" : "No (might be new allocation)")
			   << "\n";

		ASSERT(q2.GetCount() == 2);
	}

	// Stress Test 1: High volume add/remove
	{
		Cout() << "Stress Test 1: 1,000,000 Add/Drop cycles...\n";
		BiVectorRecycler<TestType, true> q;
		const int N = 1000000;

		// We will maintain a small buffer size but cycle through it many times
		// This stresses the reuse mechanism
		for(int i = 0; i < N; i++) {
			TestType* t = q.AddTail();
			t->a = i;
			if(q.GetCount() > 100) {
				ASSERT(q.Head().a == i - 100);
				q.DropHead();
			}
		}
		ASSERT(q.GetCount() == 100);
		q.Clear();
		ASSERT(q.GetCount() == 0);
	}

	// Stress Test 2: Randomized Head/Tail operations
	{
		Cout() << "Stress Test 2: Randomized operations...\n";
		BiVectorRecycler<TestType, true> q;
		int count = 0;
		SeedRandom(123);

		for(int i = 0; i < 100000; i++) {
			int op = Random(4);
			if(count == 0)
				op = op % 2; // Only add if empty

			if(op == 0) { // AddTail
				q.AddTail()->a = i;
				count++;
			}
			else if(op == 1) { // AddHead
				q.AddHead()->a = i;
				count++;
			}
			else if(op == 2) { // DropHead
				q.DropHead();
				count--;
			}
			else if(op == 3) { // DropTail
				q.DropTail();
				count--;
			}
			ASSERT(q.GetCount() == count);
		}
		Cout() << "Final count: " << q.GetCount() << "\n";
	}

	// Stress Test 3: Large Transfer
	{
		Cout() << "Stress Test 3: Large Pick Transfer...\n";
		BiVectorRecycler<TestType, true> q1;
		const int N = 50000;
		for(int i = 0; i < N; i++)
			q1.AddTail()->a = i;

		ASSERT(q1.GetCount() == N);

		BiVectorRecycler<TestType, true> q2;
		q2 = pick(q1);

		ASSERT(q1.GetCount() == 0);
		ASSERT(q2.GetCount() == N);
		ASSERT(q2.Head().a == 0);
		ASSERT(q2.Tail().a == N - 1);

		q2.Clear();
		ASSERT(q2.GetCount() == 0);
	}

	// Stress Test 4: Threaded usage

	{

		Cout() << "Stress Test 4: Threaded usage (RWMutex)...\n";

		BiVectorRecycler<TestType, true> q;

		RWMutex mutex;

		std::atomic<bool> running(true);

		CoWork co;

		const int THREADS = 8;

		for(int t = 0; t < THREADS; t++) {

			co& [&, t] {
				// Seed per thread

				SeedRandom(123 + t);

				// Wait for start signal or just go? We want contention.

				// Run for a duration or count? Let's do duration loop.

				TimeStop ts;

				while(ts.Elapsed() < 20000 && running) {

					int op = Random(4);

					if(op <= 1) { // Add

						RWMutex::WriteLock __(mutex);

						// Limit size to keep things churning

						if(q.GetCount() < 1000) {

							if(op == 0) {
								auto& o = *q.AddTail();
								o.a = Random(10000);
								for(int i = 0; i < 64; i++)
									o.payload[i] = o.a+i;
							}

							else {
								auto& o = *q.AddHead();
								o.a = Random(10000);
								for(int i = 0; i < 64; i++)
									o.payload[i] = o.a+i;
							}
						}
					}

					else { // Remove

						RWMutex::WriteLock __(mutex);

						if(q.GetCount() > 0) {

							if(op == 2)
								q.DropHead();

							else
								q.DropTail();
						}
					}

					// Occasional read test

					if(Random(10) == 0) {

						RWMutex::ReadLock __(mutex);

						if(q.GetCount() > 0) {

							int idx = Random(q.GetCount());

							auto& o = q[idx];
							ASSERT(o.a >= 0 && o.a < 10000);
							for(int i = 0; i < 64; i++) {
								ASSERT(o.payload[i] == o.a+i);
							}
						}
					}
				}
			};
		}

		// Wait for completion (CoWork destructor waits)
	}

	Cout() << "BiVectorRecycler stress tests passed\n";
}

	