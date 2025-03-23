using namespace QPI;

struct MARKETPLACE2
{
};

struct MARKETPLACE : public ContractBase
{
public:
    // Agreement status codes
    static const uint8 STATUS_PROPOSED = 0;  // Proposed but not accepted
    static const uint8 STATUS_ACCEPTED = 1;  // Accepted by both parties
    static const uint8 STATUS_PAID = 2;      // Paid by the buyer
    static const uint8 STATUS_DELIVERED = 3; // Delivered by the seller
    static const uint8 STATUS_COMPLETED = 4; // Completed and closed
    static const uint8 STATUS_DISPUTED = 5;  // In dispute
    static const uint8 STATUS_CANCELLED = 6; // Cancelled

    // Structure for creating an agreement
    struct CreateAgreement_input {
        id buyer;           // Buyer's ID
        id seller;          // Seller's ID
        uint64 amount;      // Agreed payment amount
        char description[64]; // Service/product description (fixed size)
        uint64 deadline;    // Deadline (system tick)
    };
    struct CreateAgreement_output {
        uint64 agreementId; // Created agreement ID
    };

    // Accept agreement (by seller)
    struct AcceptAgreement_input {
        uint64 agreementId;
    };
    struct AcceptAgreement_output {
        bool success;
    };

    // Make payment
    struct MakePayment_input {
        uint64 agreementId;
    };
    struct MakePayment_output {
        bool success;
    };

    // Confirm delivery
    struct ConfirmDelivery_input {
        uint64 agreementId;
    };
    struct ConfirmDelivery_output {
        bool success;
    };

    // Query agreement details
    struct GetAgreementDetails_input {
        uint64 agreementId;
    };
    struct GetAgreementDetails_output {
        id buyer;
        id seller;
        uint64 amount;
        char description[64];
        uint64 deadline;
        uint8 status;
        bool exists;
    };

private:
    // Agreement structure
    struct Agreement {
        id buyer;
        id seller;
        uint64 amount;
        char description[64];
        uint64 deadline;
        uint8 status;
    };

    // Map of agreements by ID - Using HashMap instead of Map
    HashMap<uint64, Agreement, 1024> agreements;
    uint64 nextAgreementId;

    /**
    * Create a new business agreement
    */
    PUBLIC_PROCEDURE(CreateAgreement)
        // Create new agreement with PROPOSED status
        Agreement newAgreement;
        newAgreement.buyer = input.buyer;
        newAgreement.seller = input.seller;
        newAgreement.amount = input.amount;
        
        // Copy description safely (avoiding overflows)
        for (int i = 0; i < 64; i++) {
            newAgreement.description[i] = input.description[i];
            if (input.description[i] == 0) break;
        }
        // Ensure string termination
        newAgreement.description[63] = 0;
        
        newAgreement.deadline = input.deadline;
        newAgreement.status = STATUS_PROPOSED;
        
        // Assign ID and save
        uint64 agreementId = state.nextAgreementId++;
        state.agreements.set(agreementId, newAgreement);
        
        output.agreementId = agreementId;
    _

    /**
    * Accept an agreement (only the seller)
    */
    PUBLIC_PROCEDURE(AcceptAgreement)
        output.success = false;
        
        // Verify agreement exists
        ValueT agreement;
        if (!state.agreements.get(input.agreementId, agreement)) {
            return;
        }
        
        // Only seller can accept
        if (qpi.invocator() != agreement.seller) {
            return;
        }
        
        // Can only be accepted in PROPOSED status
        if (agreement.status != STATUS_PROPOSED) {
            return;
        }
        
        // Update status
        agreement.status = STATUS_ACCEPTED;
        state.agreements.set(input.agreementId, agreement);
        
        output.success = true;
    _

    /**
    * Make payment for an agreement (only the buyer)
    */
    PUBLIC_PROCEDURE(MakePayment)
        output.success = false;
        
        // Verify agreement exists
        ValueT agreement;
        if (!state.agreements.get(input.agreementId, agreement)) {
            return;
        }
        
        // Only buyer can pay
        if (qpi.invocator() != agreement.buyer) {
            return;
        }
        
        // Can only pay in ACCEPTED status
        if (agreement.status != STATUS_ACCEPTED) {
            return;
        }
        
        // Verify correct amount
        if (qpi.invocationReward() != agreement.amount) {
            // Return funds if amount is incorrect
            qpi.transfer(qpi.invocator(), qpi.invocationReward());
            return;
        }
        
        // Update status
        agreement.status = STATUS_PAID;
        state.agreements.set(input.agreementId, agreement);
        
        output.success = true;
    _

    /**
    * Confirm delivery (by the buyer)
    */
    PUBLIC_PROCEDURE(ConfirmDelivery)
        output.success = false;
        
        // Verify agreement exists
        ValueT agreement;
        if (!state.agreements.get(input.agreementId, agreement)) {
            return;
        }
        
        // Only buyer can confirm
        if (qpi.invocator() != agreement.buyer) {
            return;
        }
        
        // Can only confirm in PAID status
        if (agreement.status != STATUS_PAID) {
            return;
        }
        
        // Transfer funds to seller
        qpi.transfer(agreement.seller, agreement.amount);
        
        // Update status
        agreement.status = STATUS_COMPLETED;
        state.agreements.set(input.agreementId, agreement);
        
        output.success = true;
    _

    /**
    * Get details of an agreement
    */
    PUBLIC_FUNCTION(GetAgreementDetails)
        output.exists = false;
        
        // Verify agreement exists
        ValueT agreement;
        if (!state.agreements.get(input.agreementId, agreement)) {
            return;
        }
        
        // Get details
        output.buyer = agreement.buyer;
        output.seller = agreement.seller;
        output.amount = agreement.amount;
        
        // Copy description safely
        for (int i = 0; i < 64; i++) {
            output.description[i] = agreement.description[i];
            if (agreement.description[i] == 0) break;
        }
        
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