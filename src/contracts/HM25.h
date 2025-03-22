using namespace QPI;

struct HM252
{
};

struct HM25 : public ContractBase
{
public:
    struct Echo_input{};
    struct Echo_output{};

    struct Burn_input{};
    struct Burn_output{};

    struct GetStats_input {};
    struct GetStats_output
    {
        uint64 numberOfEchoCalls;
        uint64 numberOfBurnCalls;
        uint64 numberOfPayments;  // Contador de pagos recibidos
    };

    // Nueva estructura para pago fijo
    struct PayFixed_input {};
    struct PayFixed_output {
        bool success;
    };

private:
    // Constante para el pago fijo
    static const uint64 FIXED_PAYMENT = 350;
    
    // Variables originales
    uint64 numberOfEchoCalls;
    uint64 numberOfBurnCalls;
    
    // Nueva variable
    uint64 numberOfPayments;

    /**
    Send back the invocation amount
    */
    PUBLIC_PROCEDURE(Echo)
        state.numberOfEchoCalls++;
        if (qpi.invocationReward() > 0)
        {
            qpi.transfer(qpi.invocator(), qpi.invocationReward());
        }
    _

    /**
    * Burn all invocation amount
    */
    PUBLIC_PROCEDURE(Burn)
        state.numberOfBurnCalls++;
        if (qpi.invocationReward() > 0)
        {
            qpi.burn(qpi.invocationReward());
        }
    _

    /**
    * Process fixed payment of 350 tokens
    */
    PUBLIC_PROCEDURE(PayFixed)
        output.success = false;
        
        // Verificar que el pago es exactamente 350 tokens
        if (qpi.invocationReward() == FIXED_PAYMENT)
        {
            // Incrementar contador de pagos
            state.numberOfPayments++;
            
            // El contrato retiene los tokens
            output.success = true;
        }
    _

    PUBLIC_FUNCTION(GetStats)
        output.numberOfBurnCalls = state.numberOfBurnCalls;
        output.numberOfEchoCalls = state.numberOfEchoCalls;
        output.numberOfPayments = state.numberOfPayments;
    _

    REGISTER_USER_FUNCTIONS_AND_PROCEDURES
        REGISTER_USER_PROCEDURE(Echo, 1);
        REGISTER_USER_PROCEDURE(Burn, 2);
        REGISTER_USER_PROCEDURE(PayFixed, 3);

        REGISTER_USER_FUNCTION(GetStats, 1);
    _

    INITIALIZE
        state.numberOfEchoCalls = 0;
        state.numberOfBurnCalls = 0;
        state.numberOfPayments = 0;
    _
};
