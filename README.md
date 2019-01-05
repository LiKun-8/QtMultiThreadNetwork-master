@Brief:		Qt multi-threaded network module
@version:	2.0.0.3
@Last modified: 2019/1/4
@Author:	Vilas Wang
@Contact:	QQ451930733
@Email:		vilas900420@gmail.com / 451930733@qq.com

Copyright © 2016-2018 Vilas Wang. All rights reserved.

** QtMultiThreadNetwork is free library licensed under the term of LGPL v3.0.
If you use QtMultiThreadNetwork or its constituent libraries, you must adhere to the terms of the license in question. **




## Detailed Description

The Qt multi-threaded network module is a wrapper of Qt Network module, and combine with thread-pool to realize multi-threaded networking.
- Multitask concurrent(Different request tasks are executed concurrently in different threads).
- Both single request and batch request mode are supported.
- Big file multi-thread downloading supported. (The thread here refers to the download channel. Download speed is faster)
- HTTP(S)/FTP protocol supported.
- Multiple request methods supported. (GET/POST/PUT/DELETE/HEAD)
- Asynchronous API.
- Thread-safe.

Note:	You must call NetworkManager::initialize() before use, and call NetworkManager::unInitialize() before application quit. 
	That must be called in the main thread.


Qt多线程网络模块是对Qt Network的封装，并且结合线程池以实现多线程网络请求，
- 多任务并发执行（不同请求任务在对应的不同线程中并发执行）
- 支持单请求任务或批处理请求两种请求模式
- 支持大文件多线程下载模式（这里的线程是指下载的通道。多个通道同时下载, 下载速度更快）
- 支持HTTP(S)/FTP协议
- 支持多种请求方式（GET/POST/PUT/DELETE/HEAD）
- 异步调用
- 所有方法线程安全

注意:	在使用之前，你必须先调用NetworkManager::initialize()，并且在程序退出前调用NetworkManager::unInitialize().
	初始化和反初始化都必须在主线程中被调用。




## How to use

Main thread：

#include "networkdef.h"
#include "networkmanager.h"
#include "networkreply.h"

NetworkManager::initialize();	//After app start.
NetworkManager::unInitialize();	//Before app quit.


## How to download a file by using Qt multi-threaded network module

RequestTask task;
task.url = QUrl("url");
task.eType = eTypeDownload;
task.bShowProgress = true;
task.strRequestArg = "save dir";
//One request corresponding to one NetworkReply object
NetworkReply *pReply = NetworkManager::globalInstance()->addRequest(task); 
if (nullptr != pReply)
{
	connect(pReply, SIGNAL(requestFinished(const RequestTask &)),
		this, SLOT(onRequestFinished(const RequestTask &)));
}


## How to download a batch of files by using Qt multi-threaded network module (such as updating application.)

connect(NetworkManager::globalInstance(), &NetworkManager::downloadProgress, this, &NetworkTool::onDownloadProgress);
connect(NetworkManager::globalInstance(), &NetworkManager::uploadProgress, this, &NetworkTool::onUploadProgress);
connect(NetworkManager::globalInstance(), &NetworkManager::batchDownloadProgress, this, &NetworkTool::onBatchDownloadProgress);
connect(NetworkManager::globalInstance(), &NetworkManager::batchUploadProgress, this, &NetworkTool::onBatchUploadProgress);

RequestTasks tasks;
RequestTask task;
foreach (const QString& strUrl, strlstUrl)
{
	task.url = QUrl(QDir::fromNativeSeparators(strUrl));
	task.eTaskType = eTypeDownload;
	task.bShowProgress = true;
	task.strRequestArg = "save dir";
	tasks.append(task);
}
//A batch of requests corresponding to one NetworkReply object
quint64 uiBatchId = 0;
NetworkReply *pReply = NetworkManager::globalInstance()->addBatchRequest(tasks, uiBatchId);
qDebug() << "uiBatchId: " << uiBatchId;
if (nullptr != pReply)
{
	connect(pReply, SIGNAL(requestFinished(const RequestTask &)),
		this, SLOT(onRequestFinished(const RequestTask &)));
}


## How to quit task(s) by using Qt multi-threaded network module

//停止一個任務
quint64 id = 0;
RequestTask task;
...
NetworkReply *pReply = NetworkManager::globalInstance()->addRequest(task); 
if (nullptr != pReply)
{
	id =  task.uiId;
	connect(pReply, SIGNAL(requestFinished(const RequestTask &)),
		this, SLOT(onRequestFinished(const RequestTask &)));
}
if(id > 0)
{
	NetworkManager::globalInstance()->stopRequest(id);
}

//停止一批任務
quint64 uiBatchId = 0;
RequestTasks tasks;
...
NetworkReply *pReply = NetworkManager::globalInstance()->addBatchRequest(tasks, uiBatchId);
if (nullptr != pReply)
{
	connect(pReply, SIGNAL(requestFinished(const RequestTask &)),
		this, SLOT(onRequestFinished(const RequestTask &)));
}
if(uiBatchId > 0)
{
	NetworkManager::globalInstance()->stopBatchRequests(uiBatchId);
}


//停止所有任務
RequestTasks tasks;
RequestTask task1;
tasks.append(task1);
RequestTask task2;
tasks.append(task2);
...
NetworkReply *pReply = NetworkManager::globalInstance()->addBatchRequest(tasks, uiBatchId);
...
NetworkManager::globalInstance()->stopAllRequest();