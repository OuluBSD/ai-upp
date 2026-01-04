#!/usr/bin/env bash

# Build:
#umk ./uppsrc ide ~/.config/u++/theide/CLANG.bm -bsH1 +NET,CURL,LCLANG,AUDIO,SYS_PORTAUDIO,DEBUG_FULL bin/theide
umk ./uppsrc ide ~/.config/u++/theide/CLANG.bm -bsH1 +NOGUI,V1,DEBUG_FULL bin/theide_console

# Run ide:
#~/tmp-ide
