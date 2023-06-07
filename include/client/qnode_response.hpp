#pragma once

#include <QNetworkReply>
#include <QJsonValue>
#include <QtCore/QtGlobal>

#if defined(WINDOWS_QCLIENT)
# define QCLIENT_EXPORT Q_DECL_EXPORT
#else
#define QCLIENT_EXPORT Q_DECL_IMPORT
#endif

namespace qiota{


	class QCLIENT_EXPORT Response: public QObject
	{
		Q_OBJECT
		public:
			Response(QNetworkReply *thereply);
			void fill();
			void error_found(QNetworkReply::NetworkError code);
		signals:
			void returned( QJsonValue data );
		private:
			QNetworkReply *reply;

	};



};
