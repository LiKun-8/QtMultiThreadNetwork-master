#include "NetworkCommonRequest.h"
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkProxy>
#include "Log4cplusWrapper.h"


NetworkCommonRequest::NetworkCommonRequest(QObject *parent /* = nullptr */)
	: NetworkRequest(parent),
	m_pNetworkManager(nullptr),
	m_pNetworkReply(nullptr)
{
}

NetworkCommonRequest::~NetworkCommonRequest()
{
	abort();
	if (m_pNetworkManager)
	{
		m_pNetworkManager->deleteLater();
		m_pNetworkManager = nullptr;
	}
}

void NetworkCommonRequest::abort()
{
	if (m_pNetworkReply)
	{
		m_bAbortManual = true;
		m_pNetworkReply->abort();
		m_pNetworkReply->deleteLater();
		m_pNetworkReply = nullptr;
	}
}

QString NetworkCommonRequest::getRequestTypeString() const
{
	QString strType;
	switch (m_request.eType)
	{
	case eTypeDownload:
		{
			strType = QStringLiteral("下载");
		}
		break;
	case eTypeMTDownload:
		{
			strType = QStringLiteral("MT下载");
		}
		break;
	case eTypeUpload:
		{
			strType = QStringLiteral("上传");
		}
		break;
	case eTypeGet:
		{
			strType = QStringLiteral("GET");
		}
		break;
	case eTypePost:
		{
			strType = QStringLiteral("POST");
		}
		break;
	case eTypePut:
		{
			strType = QStringLiteral("PUT");
		}
		break;
	case eTypeDelete:
		{
			strType = QStringLiteral("DELETE");
		}
		break;
	case eTypeHead:
		{
			strType = QStringLiteral("HEAD");
		}
		break;
	default:
		break;
	}
	return strType;
}

void NetworkCommonRequest::start()
{
	m_bAbortManual = false;

	QUrl url;
	if (!redirected())
	{
		url = m_request.url;
	}
	else
	{
		url = m_redirectUrl;
	}

	if (isFtpProxy(url.scheme()))
	{
		if (m_request.eType == eTypePost
			|| m_request.eType == eTypeDelete
			|| m_request.eType == eTypeHead)
		{
			QString strType = getRequestTypeString();
			m_strError = QStringLiteral("Unsupported FTP request type[%1], url: %2").arg(strType).arg(m_request.url.url());
			qDebug() << m_strError;
			LOG_ERROR(m_strError.toStdWString());
			emit requestFinished(false, m_strError.toUtf8());
			return;
		}
	}

	if (nullptr == m_pNetworkManager)
	{
		m_pNetworkManager = new QNetworkAccessManager;
#if 0 //test
		QNetworkProxy proxy;
		proxy.setType(QNetworkProxy::HttpProxy);
		proxy.setHostName("127.0.0.1");
		proxy.setPort(8888);
		m_pNetworkManager->setProxy(proxy);
#endif // 0
	}
	//m_pNetworkManager->connectToHost(url.host(), url.port());

	QNetworkRequest request(url);
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded;");

	auto iter = m_request.mapRawHeader.cbegin();
	for (; iter != m_request.mapRawHeader.cend(); ++iter)
	{
		request.setRawHeader(iter.key(), iter.value());
	}

#ifndef QT_NO_SSL
	if (isHttpsProxy(url.scheme()))
	{
		// 发送https请求前准备工作;
		QSslConfiguration conf = request.sslConfiguration();
		conf.setPeerVerifyMode(QSslSocket::VerifyNone);
		conf.setProtocol(QSsl::TlsV1SslV3);
		request.setSslConfiguration(conf);
	}
#endif

	if (m_request.eType == eTypeGet)
	{
		m_pNetworkReply = m_pNetworkManager->get(request);
	}
	else if (m_request.eType == eTypePost)
	{
		const QByteArray& bytes = m_request.strReqArg.toUtf8();
		request.setHeader(QNetworkRequest::ContentLengthHeader, bytes.length());

		m_pNetworkReply = m_pNetworkManager->post(request, bytes);
	}
	else if (m_request.eType == eTypePut)
	{
		const QByteArray& bytes = m_request.strReqArg.toUtf8();
		request.setHeader(QNetworkRequest::ContentLengthHeader, bytes.length());

		m_pNetworkReply = m_pNetworkManager->put(request, bytes);
	}
	else if (m_request.eType == eTypeDelete)
	{
		m_pNetworkReply = m_pNetworkManager->deleteResource(request);
	}
	else if (m_request.eType == eTypeHead)
	{
		m_pNetworkReply = m_pNetworkManager->head(request);
	}

	connect(m_pNetworkReply, SIGNAL(finished()), this, SLOT(onFinished()));
	connect(m_pNetworkReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onError(QNetworkReply::NetworkError)));
	connect(m_pNetworkManager, SIGNAL(authenticationRequired(QNetworkReply *, QAuthenticator *)),
		SLOT(onAuthenticationRequired(QNetworkReply *, QAuthenticator *)));
}

void NetworkCommonRequest::onFinished()
{
	bool bSuccess = (m_pNetworkReply->error() == QNetworkReply::NoError);
	int statusCode = m_pNetworkReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
	if (isHttpProxy(m_request.url.scheme()) || isHttpsProxy(m_request.url.scheme()))
	{
		bSuccess = bSuccess && (statusCode >= 200 && statusCode < 300);
	}
	if (!bSuccess)
	{
		if (statusCode == 301 || statusCode == 302)
		{//301,302重定向
			const QVariant& redirectionTarget = m_pNetworkReply->attribute(QNetworkRequest::RedirectionTargetAttribute);
			if (!redirectionTarget.isNull())
			{
				const QUrl& url = m_request.url;
				const QUrl& redirectUrl = url.resolved(redirectionTarget.toUrl());
				if (url != redirectUrl && m_redirectUrl != redirectUrl)
				{
					m_redirectUrl = redirectUrl;
					if (m_redirectUrl.isValid())
					{
						qDebug() << "url:" << url.toString() << "redirectUrl:" << m_redirectUrl.toString();
						LOG_INFO("url: " << url.toString().toStdWString() << "; redirectUrl:" << m_redirectUrl.toString().toStdWString());

						m_pNetworkReply->deleteLater();
						m_pNetworkReply = nullptr;

						start();
						return;
					}
				}
			}
		}
		else if (statusCode != 200 && statusCode != 0)
		{
			qDebug() << "HttpStatusCode: " << statusCode;
		}
	}

	if (!m_bAbortManual)//非调用abort()结束
	{
		if (bSuccess && m_request.eType == eTypeHead)
		{
			QString headers;
			foreach(const QByteArray& header, m_pNetworkReply->rawHeaderList())
			{
				headers.append(QString("%1 -> %2").arg(QString::fromUtf8(header))
					.arg(QString::fromUtf8(m_pNetworkReply->rawHeader(header))));
				headers.append("\n");
			}
			emit requestFinished(bSuccess, headers.toUtf8());
		}
		else
		{
			if (m_pNetworkReply->isOpen())
			{
				const QByteArray& bytes = m_pNetworkReply->readAll();
				if (!bytes.isEmpty())
				{
					m_strError.append(QString::fromUtf8(bytes));
				}
			}
			emit requestFinished(bSuccess, m_strError.toUtf8());
		}
	}
	else//调用abort()结束
	{
		emit requestFinished(bSuccess, m_strError.toUtf8());
	}

	m_pNetworkReply->deleteLater();
	m_pNetworkManager->deleteLater();
	m_pNetworkReply = nullptr;
	m_pNetworkManager = nullptr;
}

void NetworkCommonRequest::onError(QNetworkReply::NetworkError code)
{
	LOG_FUN("");
	Q_UNUSED(code);
	qDebug() << QString("Type[%1]").arg(m_request.eType) << "onError" << m_pNetworkReply->errorString();
	LOG_ERROR("[url]" << m_request.url.toString().toStdWString()
		<< "  [type]" << m_request.eType
		<< "  [error]" << m_pNetworkReply->errorString().toStdString());
	m_strError = m_pNetworkReply->errorString();
}

void NetworkCommonRequest::onAuthenticationRequired(QNetworkReply *r, QAuthenticator *a)
{
	qDebug() << __FUNCTION__ << r->readAll();
}