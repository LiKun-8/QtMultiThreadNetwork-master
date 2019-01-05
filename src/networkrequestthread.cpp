#include <QDebug>
#include <QEventLoop>
#include "NetworkRequestThread.h"
#include "NetworkRequest.h"
#include "Log4cplusWrapper.h"
#include "networkmanager.h"

NetworkRequestThread::NetworkRequestThread(const RequestTask &task, QObject *parent)
	: QObject(parent),
	m_task(task)
{
	setAutoDelete(true);
}

NetworkRequestThread::~NetworkRequestThread()
{
}

void NetworkRequestThread::run()
{
	QEventLoop loop;
	connect(this, SIGNAL(exitEventLoop()), &loop, SLOT(quit()));

	NetworkRequest *pRequest = NetworkRequestFactory::createRequestInstance(m_task.eType, m_task.bMultiDownloadMode);
	if (nullptr != pRequest)
	{
		connect(pRequest, SIGNAL(requestFinished(bool, const QByteArray&)),
			this, SLOT(onRequestFinished(bool, const QByteArray&)));

		pRequest->setRequestTask(m_task);
		pRequest->start();
	}
	else
	{
		qDebug() << QString("Unsupported type(%1) ----").arg(m_task.eType) << m_task.url.url();
		LOG_ERROR("Unsupported type(" << m_task.eType << ")  ---- " << m_task.url.url().toStdWString());

		m_task.bSuccess = false;
		emit requestFinished(m_task);
	}
	loop.exec();

	if (nullptr != pRequest)
	{
		pRequest->abort();
		pRequest->deleteLater();
	}
}

quint64 NetworkRequestThread::requsetId() const
{
	return m_task.uiId;
}

quint64 NetworkRequestThread::batchId() const
{
	return m_task.uiBatchId;
}

void NetworkRequestThread::quit()
{
	disconnect(this, SIGNAL(requestFinished(const RequestTask &)),
		NetworkManager::globalInstance(), SLOT(onRequestFinished(const RequestTask &)));
	emit exitEventLoop();
}

void NetworkRequestThread::onRequestFinished(bool bSuccess, const QByteArray& bytesContent)
{
	m_task.bSuccess = bSuccess;
	m_task.bytesContent = bytesContent;
	emit requestFinished(m_task);
}