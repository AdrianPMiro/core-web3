#ifndef MARKETPLACE_H
#define MARKETPLACE_H

#include "qpi.h"  // Include the Qubic Programming Interface definitions

// -----------------------------------------------------------------------------
// Marketplace Smart Contract
// -----------------------------------------------------------------------------

// We assume that Qubic defines a templated Array type for fixed-size arrays,
// for example: Array<uint8,128> for 128 bytes.
 
struct MarketplaceState {
    id developer;                    // Developer's Qubic address
    id client;                       // Client's Qubic address (if hired)
    uint64 fixedPrice;               // Fixed price for the service (350 QUs)
    Array<uint8,128> description;    // Service description (fixed-size array)
    uint8 serviceHired;              // 0 = available, 1 = service hired
};

struct RegisterService_input {
    Array<uint8,128> description;    // New service description
};
struct RegisterService_output {
    // No output required
};

struct Hire_input {
    // No additional fields (the payment is attached to the transaction)
};
struct Hire_output {
    // No output required
};

struct ConfirmDelivery_input {
    // No input needed
};
struct ConfirmDelivery_output {
    // No output required
};

struct GetInfo_input {
    // No input needed
};
struct GetInfo_output {
    uint64 fixedPrice;               // Fixed price (350 QUs)
    Array<uint8,128> description;    // Current service description
    id developer;                    // Registered developer address
    id client;                       // Client's address if hired
    uint8 serviceHired;              // Service status: 0 or 1
};

class Marketplace : public ContractBase {
public:
    MarketplaceState state;

    // INITIALIZE: Set initial state values on deployment
    INITIALIZE {
        state.developer = qpi.zeroId();       // No developer registered
        state.client = qpi.zeroId();          // No client registered
        state.fixedPrice = 350;               // Fixed price is 350 QUs
        state.serviceHired = 0;               // Service available
        qpi.setMemory(state.description, 0);  // Initialize description as empty
    } _

    // PUBLIC PROCEDURE: RegisterService
    // Called by the developer to register or update the service description.
    PUBLIC_PROCEDURE(RegisterService) {
        RegisterService_input* input = static_cast<RegisterService_input*>(inputData);
        // Allow registration if no developer is set, or if the caller is the developer.
        if (state.developer == qpi.zeroId() || state.developer == qpi.invocator()) {
            state.developer = qpi.invocator();  // Set developer identity
            qpi.memoryCopy(state.description, input->description);
            state.serviceHired = 0;               // Reset service status on update
        }
    } _

    // PUBLIC PROCEDURE: Hire
    // Called by a client to hire the service. Requires sending at least 350 QUs.
    PUBLIC_PROCEDURE(Hire) {
        if (state.serviceHired == 0 && qpi.invocationReward() >= state.fixedPrice && qpi.invocator() != state.developer) {
            state.client = qpi.invocator();
            state.serviceHired = 1;
            uint64 paid = qpi.invocationReward();
            // Refund any extra funds above the fixed price.
            if (paid > state.fixedPrice) {
                qpi.transfer(state.client, paid - state.fixedPrice);
            }
        } else {
            // Conditions not met: refund the full amount
            if (qpi.invocationReward() > 0) {
                qpi.transfer(qpi.invocator(), qpi.invocationReward());
            }
        }
    } _

    // PUBLIC PROCEDURE: ConfirmDelivery
    // Called by the developer to confirm delivery and release the payment.
    PUBLIC_PROCEDURE(ConfirmDelivery) {
        if (state.serviceHired == 1 && qpi.invocator() == state.developer) {
            // Release the fixed price to the developer
            qpi.transfer(state.developer, state.fixedPrice);
            // Reset service status
            state.serviceHired = 0;
            state.client = qpi.zeroId();
        }
    } _

    // PUBLIC FUNCTION: GetInfo
    // Returns the current service information.
    PUBLIC_FUNCTION(GetInfo) {
        GetInfo_output* output = static_cast<GetInfo_output*>(outputData);
        output->fixedPrice = state.fixedPrice;
        qpi.memoryCopy(output->description, state.description);
        output->developer = state.developer;
        output->client = state.client;
        output->serviceHired = state.serviceHired;
    } _

    // Register the procedures and functions for external invocation.
    REGISTER_USER_FUNCTIONS_AND_PROCEDURES
        REGISTER_USER_PROCEDURE(RegisterService, 1);
        REGISTER_USER_PROCEDURE(Hire, 2);
        REGISTER_USER_PROCEDURE(ConfirmDelivery, 3);
        REGISTER_USER_FUNCTION(GetInfo, 1);
    _
};

#endif // MARKETPLACE_H
