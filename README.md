# 
模拟Linux文件系统(c++)

 开辟100MB存储空间，编写程序simdisk对磁盘空间进行管理，以模拟Linux文件系统。
 (1)	info:  显示整个系统信息(参考Linux文件系统的系统信息)，文件可以根据用户进行读写保护。目录名和文件名支持全路径名和相对路径名，路径名各分量间用“/”隔开。
(2)	cd …:  改变目录：改变当前工作目录，目录不存在时给出出错信息。
(3)	dir …:  显示目录：显示指定目录下或当前目录下的信息，包括文件名、物理地址、保护码、文件长度、子目录等（带/s参数的dir命令，显示所有子目录）。
(4)	md …:  创建目录：在指定路径或当前路径下创建指定目录。重名时给出错信息。
(5)	rd …:  删除目录：删除指定目录下所有文件和子目录。要删目录不空时，要给出提示是否要删除。
(6)	newfile …:  建立文件。
(7)	cat …:  打开文件。
(8)	copy …:  拷贝文件，除支持模拟Linux文件系统内部的文件拷贝外，还支持host文件系统与模拟Linux文件系统间的文件拷贝，host文件系统的文件命名为<host>…，如：将windows下D：盘的文件\data\sample\test.txt文件拷贝到模拟Linux文件系统中的/test/data目录，windows下D：盘的当前目录为D：\data，则使用命令：
simdisk copy <host>D：\data\sample\test.txt /test/data
或者：simdisk copy <host>D：sample\test.txt /test/data
(9)	del …:  删除文件：删除指定文件，不存在时给出出错信息。
