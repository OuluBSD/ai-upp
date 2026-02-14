print("--- Forex Verification Script ---")

print("Scraping Live Rates...")
cli("forex scrape")

print(chr(10) + "Upcoming Calendar Events:")
cli("forex calendar")

print(chr(10) + "Live Rates:")
cli("forex rates")

print(chr(10) + "Recent Trades:")
cli("forex trades")

print(chr(10) + "Verification Complete.")
_exit(0)