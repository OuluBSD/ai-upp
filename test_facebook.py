print("--- Facebook Verification Script ---")

# 1. Scrape Initial Data
print("Step 1: Scrape FB Feed and Friends (takes time)...")
cli("facebook scrape")

# 2. List Friends
print(chr(10) + "Step 2: Current Friends List:")
cli("facebook friends")

# 3. Dump specific friend feed (index 0 if exists)
print(chr(10) + "Step 3: Dumping Feed for Friend Index 0...")
cli("facebook friend-feed 0")

print(chr(10) + "Verification Complete.")
_exit(0)
