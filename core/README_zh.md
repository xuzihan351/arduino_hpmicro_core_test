# arduino core说明
arduino core包含编译arduino镜像所需要使用的源代码文件，头文件，链接脚本，以及下载程序到开发板所需使用的脚本文件等。
## 使用方式
1. arduino识别开发板需要将目录按照一定的目录格式进行打包成zip文件。顶层目录为：{maintainer}，然后是{version},然后是对应soc目录下的文档。例如应当存在以下文件：hpmicroduino/1.5.0/platform.txt。
2. 将压缩包的实际大小和sha256sum值写入开发板描述文档中(json),填入相应字段，具体填写要求见：[Package index specification](https://arduino.github.io/arduino-cli/0.35/package_index_json-specification)
3. 将压缩包上传到服务器中，并将获取压缩包的url地址按要求填写到开发板描述文档中。[Package index specification](https://arduino.github.io/arduino-cli/0.35/package_index_json-specification)