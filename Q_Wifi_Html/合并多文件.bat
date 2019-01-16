rem hbin将第一个文件融合到第二个文件中
rem cat将第一个文件联结到第二个文件之后
rem 1:4096,2:8192,4:16384,8:32768,16:65535,22:90112

del *.bin
del *.obj

fsutil file createnew logo.obj 16384
hbin logo.png logo.obj

fsutil file createnew zepto.obj 90112
hbin zepto.js zepto.obj

fsutil file createnew user.obj 16384
hbin user.js user.obj

fsutil file createnew style.obj 16384
hbin style.css style.obj

fsutil file createnew default.obj 8192
hbin default.html default.obj

fsutil file createnew wifi.obj 8192
hbin wifi.html wifi.obj

fsutil file createnew ap.obj 8192
hbin ap.html ap.obj

fsutil file createnew gpio.obj 8192
hbin gpio.html gpio.obj

fsutil file createnew msg.obj 8192
hbin msg.html msg.obj

fsutil file createnew vars.obj 8192
hbin vars.html vars.obj

fsutil file createnew misc.obj 8192
hbin misc.html misc.obj

fsutil file createnew html.bin 0
cat logo.obj >> html.bin
cat zepto.obj >> html.bin
cat user.obj >> html.bin
cat style.obj >> html.bin
cat default.obj >> html.bin
cat wifi.obj >> html.bin
cat ap.obj >> html.bin
cat gpio.obj >> html.bin
cat msg.obj >> html.bin
cat vars.obj >> html.bin
cat misc.obj >> html.bin

del *.obj