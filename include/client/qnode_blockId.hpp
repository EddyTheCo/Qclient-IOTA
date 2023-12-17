#pragma once

#include"block/carray.hpp"
#include"client/qnode_response.hpp"
#include <QJsonValue>
namespace qiota{


	class QCLIENT_EXPORT Node_blockID : public QObject
	{
		Q_OBJECT
		public:
            Node_blockID(){}
			Node_blockID(Response*);
			qblocks::Block_ID id;
			public slots:
				void fill(QJsonValue data);
		signals:
			void finished(void);

		private:
			Response* response_;
	};

}
