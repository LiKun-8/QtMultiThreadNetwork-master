#include "NetworkRequest.h"
#include "NetworkDownloadRequest.h"
#include "NetworkUploadRequest.h"
#include "NetworkCommonRequest.h"
#include "NetworkMTDownloadRequest.h"


NetworkRequest *NetworkRequestFactory::createRequestInstance(const RequestType& eType, bool bBigFileMode, QObject *parent)
{
	NetworkRequest *pRequest = nullptr;
	switch (eType)
	{
	case eTypeDownload:
		{
			if (bBigFileMode)
			{
				pRequest = new NetworkMTDownloadRequest(parent);
			}
			else
			{
				pRequest = new NetworkDownloadRequest(parent);
			}
		}
		break;
	case eTypeUpload:
		{
			pRequest = new NetworkUploadRequest(parent);
		}
		break;
	case eTypePost:
	case eTypeGet:
	case eTypePut:
	case eTypeDelete:
	case eTypeHead:
		{
			pRequest = new NetworkCommonRequest(parent);
		}
		break;
	/*New type add to here*/
	default:
		break;
	}
	return pRequest;
}