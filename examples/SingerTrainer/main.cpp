#include "SingerTrainer.h"

using namespace Upp;

#ifdef flagGUI

SingerTrainer::SingerTrainer()
{
	Title("SingerTrainer").Sizeable().Zoomable();
	SetRect(0, 0, 1024, 768);

	Add(main_splitter.HSizePos().VSizePos());
	
	main_splitter.Horz(history, right_splitter);
	main_splitter.SetPos(2000); // 20% Sidebar

	right_splitter.Vert(draw_area, bottom_splitter);
	
		bottom_splitter.Horz(stats_area, notes);
		
				stats_area.Add(stats_label.HSizePos().TopPos(0, 30));
		
				stats_label.SetText("Accuracy: 0%");
		
				stats_label.SetAlign(ALIGN_CENTER);
		
				
		
				stats_area.Add(accuracy_meter.HSizePos().TopPos(30, 20));
		
				accuracy_meter.Set(0, 100);
		
				
		
				stats_area.Add(start_button.HSizePos().TopPos(60, 30));
		
				start_button.SetLabel("Start Training");
		
				
		
				stats_area.Add(user_input_slider.HSizePos().TopPos(100, 30));
		
				user_input_slider.Range(700); // 0.0 to 7.0 for modes
		
				
		
				history.AddColumn("Date");
		
				history.AddColumn("Success %");
		
			
		
				draw_area.SetEngine(engine);
	scripting.SetEngine(engine);
		
			
		
				scripting.RegisterCtrl("history", history);
		
				scripting.RegisterCtrl("notes", notes);
		
				scripting.RegisterCtrl("main_splitter", main_splitter);
		
				scripting.RegisterCtrl("start_button", start_button);
		
				scripting.RegisterCtrl("plotter", draw_area);
		
			
		
				BIND_ACTION(start_button, "on_start_training", scripting);
		
			
		
				PostCallback(THISBACK(Initialize));
		
			}
		
			
		
			SingerTrainer::~SingerTrainer()
		
			{
		
				UninitializeDeep();
		
			}
		
			
		
			void SingerTrainer::Initialize()
		
			{
		
				tc.Set(-1000 / 60, THISBACK(Data));
		
				
		
				String init_script = LoadFile(GetExeDirFile("init.py"));
		
				if (!init_script.IsEmpty()) {
		
					scripting.ExecuteScript(init_script);
		
				}
		
			}
		
			
		
			void SingerTrainer::UninitializeDeep()
		
			{
		
				tc.Kill();
		
			}
		
			
		
			void SingerTrainer::Data()
		
			{
		
				double dt = 1.0 / 60.0;
		
				
		
					// Simulate user input for now: 440Hz, and slider value for mode
		
				
		
					engine.SetUserInput(440.0, (double)user_input_slider.GetData() / 100.0);
		
				
		
					engine.Update(dt);
		
				
		
				
		
				
		
				double acc = engine.GetAccuracy();
		
				accuracy_meter.Set((int)acc);
		
				stats_label.SetText(Format("Accuracy: %.1f%%", acc));
		
				
		
				draw_area.Refresh();
		
				
		
				scripting.ExecuteScript(String().Cat() << "on_frame_update(" << dt << ")\n");
		
			}
		
			

GUI_APP_MAIN
{
	SingerTrainer().Run();
}

#else

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT | LOG_FILE);
	Cout() << "SingerTrainer Headless Mode\n";
	
	int64 start_time = msecs();
	while(msecs() - start_time < 5000) {
		Cout() << "Processing... " << (msecs() - start_time) / 1000.0 << "s\r";
		Sleep(100);
	}
	Cout() << "\nFinished.\n";
}

#endif