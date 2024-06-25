# arduino工具包说明
arduino工具包包含工具链、openocd等工具，用于编译程序和下载等步骤。目前使用的是sdk_env中的工具。
## 使用工具包的方式
1. arduino识别工具包需要将工具包按照一定的目录格式进行打包成zip文件。例如在windows下,解压后目录1.5.0/win/内为sdk_env的根目录。
2. 将压缩包的实际大小和sha256sum值写入开发板描述文档中(json),填入相应字段，具体填写要求见：[Package index specification](https://arduino.github.io/arduino-cli/0.35/package_index_json-specification)
3. 将压缩包上传到服务器中，并将获取压缩包的url地址按要求填写到开发板描述文档中。[Package index specification](https://arduino.github.io/arduino-cli/0.35/package_index_json-specification)