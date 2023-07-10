#pragma once

#include"block/qblock.hpp"
#include"client/qnode_response.hpp"
#include <QJsonValue>
namespace qiota{


	class QCLIENT_EXPORT Node_tips : public QObject
	{
		Q_OBJECT
		public:
			Node_tips(Response*);
			std::vector<qblocks::Block_ID> tips;
			public slots:
				void fill(QJsonValue data);
		signals:
			void finished(void);

		private:
			Response* response_;

	};

}
