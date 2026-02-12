print("Starting News Test...")
wait_time(2.0)
tab = find("Main/Tabs/News")
if not tab:
    tab = find("News")
tab.click()
wait_time(3.0)
btn = find("BtnRefresh")
if not btn:
    btn = find("Refresh")
btn.click()
loop = 0
count = 0
while loop < 40:
    wait_time(10.0)
    ui = dump_ui()
    count = 0
    if "ZeroHedge" in ui:
        count = count + 1
    if "NaturalNews" in ui:
        count = count + 1
    if "GlobalResearch" in ui:
        count = count + 1
    if "HackerNews" in ui:
        count = count + 1
    if "ForexFactory" in ui:
        count = count + 1
    if "FXStreet" in ui:
        count = count + 1
    if "Investing" in ui:
        count = count + 1
    if "Discord" in ui:
        count = count + 1
    print("Progress: " + str(count) + "/8")
    if count == 8:
        loop = 100
    loop = loop + 1
print("Final count: " + str(count))
_exit(0)
