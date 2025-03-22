using namespace QPI;

struct HM252
{
};

struct HM25 : public ContractBase
{
public:
    // Estados del contrato como uint8
    static const uint8 STATUS_PENDING = 0;
    static const uint8 STATUS_FUNDED = 1;
    static const uint8 STATUS_DELIVERED = 2;
    static const uint8 STATUS_APPROVED = 3;
    static const uint8 STATUS_REJECTED = 4;

    // Estructuras existentes
    struct Echo_input{};
    struct Echo_output{};

    struct Burn_input{};
    struct Burn_output{};

    struct GetStats_input {};
    struct GetStats_output
    {
        uint64 numberOfEchoCalls;
        uint64 numberOfBurnCalls;
    };

    // Nueva estructura para crear contrato
    struct CreateContract_input {
        Address clientAddress;
        Address developerAddress;
        uint64 paymentAmount;
    };
    struct CreateContract_output {};

    // Para obtener detalles del contrato
    struct GetContractDetails_input {};
    struct GetContractDetails_output {
        Address clientAddress;
        Address developerAddress;
        uint64 paymentAmount;
        uint8 status;
    };

private:
    // Variables existentes
    uint64 numberOfEchoCalls;
    uint64 numberOfBurnCalls;
    
    // Variables nuevas
    Address clientAddress;
    Address developerAddress;
    uint64 paymentAmount;
    uint8 status;

    /**
    Send back the invocation amount
    */
    PUBLIC_PROCEDURE(Echo)
        numberOfEchoCalls++;
        if (qpi.invocationReward() > 0)
        {
            qpi.transfer(qpi.invocator(), qpi.invocationReward());
        }
    _

    /**
    * Burn all invocation amount
    */
    PUBLIC_PROCEDURE(Burn)
        numberOfBurnCalls++;
        if (qpi.invocationReward() > 0)
        {
            qpi.burn(qpi.invocationReward());
        }
    _

    PUBLIC_FUNCTION(GetStats)
        output.numberOfBurnCalls = numberOfBurnCalls;
        output.numberOfEchoCalls = numberOfEchoCalls;
    _

    /**
    * Create a new contract between client and developer
    */
    PUBLIC_PROCEDURE(CreateContract)
        // Solo se puede configurar una vez
        if (status != STATUS_PENDING) {
            return;
        }
        
        clientAddress = input.clientAddress;
        developerAddress = input.developerAddress;
        paymentAmount = input.paymentAmount;
        status = STATUS_PENDING;
    _

    /**
    * Get contract details
    */
    PUBLIC_FUNCTION(GetContractDetails)
        output.clientAddress = clientAddress;
        output.developerAddress = developerAddress;
        output.paymentAmount = paymentAmount;
        output.status = status;
    _

    REGISTER_USER_FUNCTIONS_AND_PROCEDURES
        REGISTER_USER_PROCEDURE(Echo, 1);
        REGISTER_USER_PROCEDURE(Burn, 2);
        REGISTER_USER_PROCEDURE(CreateContract, 3);

        REGISTER_USER_FUNCTION(GetStats, 1);
        REGISTER_USER_FUNCTION(GetContractDetails, 2);
    _

    INITIALIZE
        numberOfEchoCalls = 0;
        numberOfBurnCalls = 0;
        status = STATUS_PENDING;
    _
};
