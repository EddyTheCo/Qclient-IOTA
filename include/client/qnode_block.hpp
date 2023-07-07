#pragma once

#include"block/qblock.hpp"
#include"client/qnode_response.hpp"
#include <QJsonValue>
namespace qiota{


	class QCLIENT_EXPORT Node_block : public QObject
	{
		Q_OBJECT
		public:
			Node_block(Response*);
            Node_block(const qblocks::Block &block_m):block_(block_m){}
			qblocks::Block block_;
			qblocks::c_array ready(void)const;

			public slots:
				void fill(QJsonValue data);
			void set_pv(const quint8& pv);
			void set_parents(const std::vector<qblocks::Block_ID> &parents_m);
			void set_nonce(const quint64& nonce_m);


		signals:
			void finished(void);

		private:
			Response* response_;
	};

}
