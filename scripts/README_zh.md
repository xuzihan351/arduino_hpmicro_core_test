此目录下存放用于打包发布的脚本

generate_zip.py:
此脚本会生成一个zip文件和一个json文件，并将其大小和sha256的值写入json文件中。将zip文件放到服务器，并将其url地址填写到json文件中，再将json文件添加到服务器中，并将其url填入Arduino IDE中，Arduino IDE即可根据json文件中的内容自动下载zip包。调试时可以使用tools/hfs.exe创建一个本地的文件服务器。

example:
 python.exe .\generate_zip.py  --name=arduino_hpm5300evk --soc=HPM5361 --boards=hpm5300evk --output=C:\Users\XJ0082\Desktop\hpmicro_sw\arduino_test\arduino_hpm5300evk.zip --json_output=C:\Users\XJ0082\Desktop\hpmicro_sw\arduino_test\arduino_hpm5300evk.json --input=C:\Users\XJ0082\Desktop\hpmicro_sw\arduino_hpmicro\core\hpm5300