import json
import os
import shutil
import tempfile
import time
import unittest
import threading
from pathlib import Path
from pysrc.bin.mcp_generalist import GeneralistRuntime, GeneralistMcpServer

class TestMcpGeneralist(unittest.TestCase):
    def setUp(self):
        self.test_dir = Path(tempfile.mkdtemp())
        self.runtime = GeneralistRuntime(self.test_dir)
        self.server = GeneralistMcpServer(self.runtime)

    def tearDown(self):
        shutil.rmtree(self.test_dir)

    def test_worker_flow(self):
        # 1. Register worker
        repo = "/tmp/repo1"
        # Force disable FIFO for this test to avoid hangs in select/mkfifo
        original_mkfifo = getattr(os, "mkfifo", None)
        if original_mkfifo:
            delattr(os, "mkfifo")
        
        try:
            reg = self.runtime.register_worker(repo, 1, "Testing")
            ident = reg["identity"]
            
            # 2. Start blocking listen in thread
            listen_result = {}
            def _listen():
                print("DEBUG: _listen thread starting")
                try:
                    listen_result["task"] = self.runtime.worker_listen(ident, timeout=5.0)
                    print(f"DEBUG: _listen thread finished with {listen_result['task']}")
                except Exception as e:
                    print(f"DEBUG: _listen thread crashed with {e}")
            
            t = threading.Thread(target=_listen)
            t.daemon = True
            t.start()
            
            # 3. Planner assigns task
            time.sleep(1) # Ensure listener is ready
            assign_res = self.runtime.assign(ident, "Say hello")
            task_id = assign_res["task_id"]
            
            t.join(timeout=5)
            
            # 4. Verify unblocked and got task
            self.assertEqual(listen_result["task"]["task"], "Say hello")
            self.assertEqual(listen_result["task"]["status"], "claimed")
            
            # 5. Worker reports
            self.runtime.report_task(task_id, "completed", "Hello from worker")
            
            # 6. Planner checks status
            status = self.runtime.status(task_id)
            self.assertEqual(status["report"]["output"], "Hello from worker")
        finally:
            if original_mkfifo:
                setattr(os, "mkfifo", original_mkfifo)

    def test_list_workers_liveness(self):
        # This test checks if we can detect liveness. 
        # Since we use file locking, we need a real file open in another thread/process.
        repo = "/tmp/repo2"
        reg = self.runtime.register_worker(repo, 2)
        ident = reg["identity"]
        
        # Initially not listening
        workers = self.runtime.list_workers()["workers"]
        self.assertFalse(workers[0]["listening"])
        
        # Mock listening via tool call logic in thread
        def _block():
            self.runtime.worker_listen(ident, timeout=2.0)
            
        t = threading.Thread(target=_block)
        t.start()
        time.sleep(0.5)
        
        workers = self.runtime.list_workers()["workers"]
        self.assertTrue(any(w["listening"] for w in workers if w["identity"] == ident))
        
        t.join()

    def test_get_identity(self):
        res = self.runtime.get_identity("/tmp/repo", 5, "My Title")
        self.assertEqual(res["identity"], "/tmp/repo #5 (My Title)")

    def test_help_tools(self):
        # Planner help
        p_res = self.runtime.planner_help()
        self.assertIn("summary", p_res)
        self.assertIn("workflow", p_res)
        
        # Worker help
        w_res = self.runtime.worker_help()
        self.assertIn("onboarding", w_res)

    def test_standard_mcp_discovery(self):
        # Test tools/list
        res = self.server.handle({"jsonrpc": "2.0", "id": 1, "method": "tools/list"})
        tools = [t["name"] for t in res["result"]["tools"]]
        self.assertIn("generalist_register_worker", tools)
        self.assertIn("generalist_get_identity", tools)
        self.assertIn("generalist_get_version", tools)
        self.assertIn("generalist_listen", tools)

if __name__ == "__main__":
    unittest.main()
