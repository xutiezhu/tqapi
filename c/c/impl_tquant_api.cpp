#include <sstream>
#include <unordered_set>
#include "tquant_api.h"
#include "myutils/stringutils.h"
#include "myutils/mprpc.h"
#include "myutils/socket_connection.h"
#include "impl_tquant_api.h"
#include "impl_data_api.h"
#include "impl_trade_api.h"

namespace tquant { namespace api { namespace impl {

    using namespace ::mprpc;
    using namespace ::tquant::api;


    class TQuantApiImpl : public TQuantApi, public MpRpcClient_Callback {
        friend DataApiImpl;
        friend TradeApiImpl;
    public:
        TQuantApiImpl(const char* addr) {
            auto conn = make_shared<SocketConnection>();
            m_client = new MpRpcClient(conn);
            m_client->connect(addr, this);
            m_dapi = new DataApiImpl(this->m_client);
            m_tapi = new TradeApiImpl(this->m_client);
        }

        virtual ~TQuantApiImpl() override {
            delete m_dapi;
            delete m_tapi;
            delete m_client;
        }

        virtual TradeApi* trade_api() { return m_tapi; }

        virtual DataApi*  data_api()  { return m_dapi; }

        virtual void on_connected() {}

        virtual void on_disconnected() {}

        virtual void on_notification(shared_ptr<MpRpcMessage> rpcmsg) {
            if (strncmp(rpcmsg->method.c_str(), "dapi.", 5) == 0) {
                m_dapi->on_notification(rpcmsg);
            }
            else if (strncmp(rpcmsg->method.c_str(), "tapi.", 5) == 0) {
                m_tapi->on_notification(rpcmsg);
            }
            else  {
                m_dapi->on_notification(rpcmsg);
                m_tapi->on_notification(rpcmsg);
            }
        }
        virtual void on_call_result(int callid, shared_ptr<MpRpcMessage> cr) {}
    private:
        MpRpcClient*  m_client;
        DataApiImpl*    m_dapi;
        TradeApiImpl*   m_tapi;
    };

} } }

namespace tquant { namespace api {

    TQuantApi* TQuantApi::create(const char* addr)
    {
        return new impl::TQuantApiImpl(addr);
    }

} }
