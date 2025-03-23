using namespace QPI;

struct BusinessAgreement : public ContractBase
{
public:
    // Estados del acuerdo
    static const uint8 STATUS_PROPOSED = 0;  // Propuesto pero no aceptado
    static const uint8 STATUS_ACCEPTED = 1;  // Aceptado por ambas partes
    static const uint8 STATUS_PAID = 2;      // Pagado por el comprador
    static const uint8 STATUS_DELIVERED = 3; // Entregado por el vendedor
    static const uint8 STATUS_COMPLETED = 4; // Completado y cerrado
    static const uint8 STATUS_DISPUTED = 5;  // En disputa
    static const uint8 STATUS_CANCELLED = 6; // Cancelado

    // Estructura para crear un acuerdo
    struct CreateAgreement_input {
        Address buyer;         // Dirección del comprador
        Address seller;        // Dirección del vendedor
        uint64 amount;         // Cantidad acordada para el pago
        String description;    // Descripción del servicio/producto
        uint64 deadline;       // Plazo límite (timestamp)
    };
    struct CreateAgreement_output {
        uint64 agreementId;    // ID del acuerdo creado
    };

    // Aceptar acuerdo (por el vendedor)
    struct AcceptAgreement_input {
        uint64 agreementId;
    };
    struct AcceptAgreement_output {
        bool success;
    };

    // Realizar pago
    struct MakePayment_input {
        uint64 agreementId;
    };
    struct MakePayment_output {
        bool success;
    };

    // Confirmar entrega
    struct ConfirmDelivery_input {
        uint64 agreementId;
    };
    struct ConfirmDelivery_output {
        bool success;
    };

    // Consultar detalles del acuerdo
    struct GetAgreementDetails_input {
        uint64 agreementId;
    };
    struct GetAgreementDetails_output {
        Address buyer;
        Address seller;
        uint64 amount;
        String description;
        uint64 deadline;
        uint8 status;
        bool exists;
    };

private:
    // Estructura de un acuerdo
    struct Agreement {
        Address buyer;
        Address seller;
        uint64 amount;
        String description;
        uint64 deadline;
        uint8 status;
    };

    // Mapa de acuerdos por ID
    Map<uint64, Agreement> agreements;
    uint64 nextAgreementId;

    /**
    * Crear un nuevo acuerdo comercial
    */
    PUBLIC_PROCEDURE(CreateAgreement)
        // Crear nuevo acuerdo con estado PROPOSED
        Agreement agreement;
        agreement.buyer = input.buyer;
        agreement.seller = input.seller;
        agreement.amount = input.amount;
        agreement.description = input.description;
        agreement.deadline = input.deadline;
        agreement.status = STATUS_PROPOSED;
        
        // Asignar ID y guardar
        uint64 agreementId = state.nextAgreementId++;
        state.agreements[agreementId] = agreement;
        
        output.agreementId = agreementId;
    _

    /**
    * Aceptar un acuerdo (solo el vendedor)
    */
    PUBLIC_PROCEDURE(AcceptAgreement)
        output.success = false;
        
        // Verificar que el acuerdo existe
        if (!state.agreements.contains(input.agreementId)) {
            return;
        }
        
        Agreement& agreement = state.agreements[input.agreementId];
        
        // Solo el vendedor puede aceptar
        if (qpi.invocator() != agreement.seller) {
            return;
        }
        
        // Solo se puede aceptar en estado PROPOSED
        if (agreement.status != STATUS_PROPOSED) {
            return;
        }
        
        // Actualizar estado
        agreement.status = STATUS_ACCEPTED;
        output.success = true;
    _

    /**
    * Realizar pago por un acuerdo (solo el comprador)
    */
    PUBLIC_PROCEDURE(MakePayment)
        output.success = false;
        
        // Verificar que el acuerdo existe
        if (!state.agreements.contains(input.agreementId)) {
            return;
        }
        
        Agreement& agreement = state.agreements[input.agreementId];
        
        // Solo el comprador puede pagar
        if (qpi.invocator() != agreement.buyer) {
            return;
        }
        
        // Solo se puede pagar en estado ACCEPTED
        if (agreement.status != STATUS_ACCEPTED) {
            return;
        }
        
        // Verificar monto correcto
        if (qpi.invocationReward() != agreement.amount) {
            // Devolver fondos si el monto es incorrecto
            qpi.transfer(qpi.invocator(), qpi.invocationReward());
            return;
        }
        
        // Actualizar estado
        agreement.status = STATUS_PAID;
        output.success = true;
    _

    /**
    * Confirmar entrega (por el comprador)
    */
    PUBLIC_PROCEDURE(ConfirmDelivery)
        output.success = false;
        
        // Verificar que el acuerdo existe
        if (!state.agreements.contains(input.agreementId)) {
            return;
        }
        
        Agreement& agreement = state.agreements[input.agreementId];
        
        // Solo el comprador puede confirmar
        if (qpi.invocator() != agreement.buyer) {
            return;
        }
        
        // Solo se puede confirmar en estado PAID
        if (agreement.status != STATUS_PAID) {
            return;
        }
        
        // Transferir fondos al vendedor
        qpi.transfer(agreement.seller, agreement.amount);
        
        // Actualizar estado
        agreement.status = STATUS_COMPLETED;
        output.success = true;
    _

    /**
    * Obtener detalles de un acuerdo
    */
    PUBLIC_FUNCTION(GetAgreementDetails)
        output.exists = false;
        
        // Verificar que el acuerdo existe
        if (!state.agreements.contains(input.agreementId)) {
            return;
        }
        
        // Obtener detalles
        Agreement& agreement = state.agreements[input.agreementId];
        output.buyer = agreement.buyer;
        output.seller = agreement.seller;
        output.amount = agreement.amount;
        output.description = agreement.description;
        output.deadline = agreement.deadline;
        output.status = agreement.status;
        output.exists = true;
    _

    REGISTER_USER_FUNCTIONS_AND_PROCEDURES
        REGISTER_USER_PROCEDURE(CreateAgreement, 1);
        REGISTER_USER_PROCEDURE(AcceptAgreement, 2);
        REGISTER_USER_PROCEDURE(MakePayment, 3);
        REGISTER_USER_PROCEDURE(ConfirmDelivery, 4);

        REGISTER_USER_FUNCTION(GetAgreementDetails, 1);
    _

    INITIALIZE
        state.nextAgreementId = 1;
    _
};