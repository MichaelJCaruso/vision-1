#ifndef Vca_VSSHConnectionFactory_Interface
#define Vca_VSSHConnectionFactory_Interface

/************************
 *****  Components  *****
 ************************/

#include "Vca_VRolePlayer.h"

#include "V_VBlob.h"

#include "Vca_IBSConsumerClient.h"
#include "Vca_IBSProducerClient.h"
#include "Vca_IPipeSourceClient.h"
#include "Vca_ISSHConnectionFactory.h"

#include "Vca_VPipeSource.h"

/**************************
 *****  Declarations  *****
 **************************/

#include "Vca_VDeviceFactory.h"

/*************************
 *****  Definitions  *****
 *************************/

namespace Vca {
    class Vca_API VSSHConnectionFactory : public VRolePlayer {
        DECLARE_CONCRETE_RTTLITE (VSSHConnectionFactory, VRolePlayer);

    //  Aliases
    public:
        typedef ISSHConnectionFactory ThisInterface;

    //  Factory
    public:
        typedef ThisClass Factory;
        typedef Reference FactoryReference;

    //  Pipe Source
    public:
        class Vca_API PipeSource : public VPipeSource {
            DECLARE_CONCRETE_RTTLITE (PipeSource, VPipeSource);

        //  Construction
        public:
            PipeSource (
                Factory *pFactory,
                VString const &rHost, U32 xPort,
                VString const &rUsername, VString const &rPassword,
                VString const &rCommand, bool bErrorChannel, bool bNoDelay
            );

        //  Destruction
        private:
            ~PipeSource ();

        //  Access/Query
        public:
            VString const &host () const {
                return m_iHost;
            }
            U32 port () const {
                return m_xPort;
            }
            VString const &username () const {
                return m_iUsername;
            }
            VString const &password () const {
                return m_iPassword;
            }

            VString const &command () const {
                return m_iCommand;
            }
            Factory *factory () const {
                return m_pFactory;
            }

        //  Implementation
        private:
            virtual void supply_(IPipeSourceClient *pClient) OVERRIDE;

        //  State
        private:
            Factory::Reference  const   m_pFactory;
            VString             const   m_iHost;
            U32                 const   m_xPort;
            VString             const   m_iUsername;
            VString             const   m_iPassword;
            VString             const   m_iCommand;
            bool                const   m_bErrorChannel;
            bool                const   m_bNoDelay;
        };

    //  Transaction
    public:
        class Transaction;

    //  Construction
    private:
        VSSHConnectionFactory (VDeviceFactory *pDeviceFactory);

    //  Destruction
    private:
        ~VSSHConnectionFactory ();

    //  Cohort Index
    private:
        static VCohortIndex const &CohortIndex ();

    //  Cohort Instance
    public:
        static bool Supply (Reference &rpInstance);
        static bool Supply (ThisInterface::Reference &rpRole);

    //  Base Roles
    public:
        using BaseClass::getRole;

    //  ISSHConnectionFactory Role
    private:
        VRole<ThisClass,ISSHConnectionFactory> m_pISSHConnectionFactory;
    public:
        void getRole (ISSHConnectionFactory::Reference &rpRole) {
            m_pISSHConnectionFactory.getRole (rpRole);
        }

    //  ISSHConnectionFactory Methods
    public:
        /**
         * @param bNoDelay currently ignored; STDERR is sent to the bit bucket.
         */
        void MakeSSHConnection (
            ISSHConnectionFactory*      pRole,
            IPipeSourceClient*          pClient,
            VString const&              rHost,
            VString const&              rUsername,
            VString const&              rPassword,
            VString const&              rCommand,
            bool                        bErrorChannel
        );

    //  Local Use
    public:
        /**
         * @param bNoDelay currently ignored; STDERR is sent to the bit bucket.
         */
        static bool Supply (
            IPipeSourceClient *pClient,
            VString const &rHostname,
            VString const &rUserName,
            VString const &rPassword,
            VString const &rCommand,
            bool bErrorChannel,
            bool bNoDelay,
            U32 xPort = 22
        );
        /**
         * @param bNoDelay currently ignored; STDERR is sent to the bit bucket.
         */
        bool supply (
            IPipeSourceClient *pClient,
            VString const &rHostname,
            VString const &rUserName,
            VString const &rPassword,
            VString const &rCommand,
            bool bErrorChannel,
            bool bNoDelay,
            U32 xPort = 22
        );

        /**
         * @param bNoDelay currently ignored; STDERR is sent to the bit bucket.
         */
        static bool Supply (
            PipeSource::Reference &rpCPS,
            VString const &rHostname,
            VString const &rUsername,
            VString const &rPassword,
            VString const &rCommand,
            bool bErrorChannel,
            bool bNoDelay,
            U32 xPort = 22
        );
        /**
         * @param bNoDelay currently ignored; STDERR is sent to the bit bucket.
         */
        bool supply (
            PipeSource::Reference &rpCPS,
            VString const &rHostname,
            VString const &rUsername,
            VString const &rPassword,
            VString const &rCommand,
            bool bErrorChannel,
            bool bNoDelay,
            U32 xPort = 22
        );

    //  State
    protected:
        VDeviceFactory::Reference const m_pDeviceFactory;

    };
}


#endif
