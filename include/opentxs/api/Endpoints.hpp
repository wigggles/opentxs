// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_ENDPOINTS_HPP
#define OPENTXS_API_ENDPOINTS_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include <string>

#ifdef SWIG
// clang-format off
%rename(ZMQEndpoints) opentxs::api::Endpoints;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace api
{
class Endpoints
{
public:
    /** Account balance update notifications
     *
     *  A subscribe socket can connect to this endpoint to be notified when
     *  any account balance changes.
     *
     *  Messages bodies consist of two frames.
     *   * The first frame contains the account ID as a serialized string
     *   * The second frame contains the account balance as an Amount
     *
     *  This endpoint is active for all session types.
     */
    OPENTXS_EXPORT virtual std::string AccountUpdate() const noexcept = 0;

    /** Blockchain reorg notifications
     *
     *  A subscribe socket can connect to this endpoint to be notified when
     *  any active blockchain detects a chain reorg
     *
     *  Messages bodies consist of three frames.
     *   * The first frame contains the chain type
     *   * The second frame contains the height of the common parent block
     *   * The second frame contains the hash of the common parent block
     *
     *  This endpoint is active for client sessions only.
     */
    OPENTXS_EXPORT virtual std::string BlockchainReorg() const noexcept = 0;

    /** Connection state notifications
     *
     *  A subscribe socket can connect to this endpoint to be notified when
     *  any server connection state changes.
     *
     *  Connection state is a boolean value that is true if the connection
     *  has sent at least one message and has not timed out waiting for a reply.
     *
     *  Messages bodies consist of two frames.
     *   * The first frame contains the ID of the notary on the other side of
     *     the connection as a serialized string
     *   * The second frame contains the state as a bool
     *
     *  This endpoint is active for client sessions only.
     */
    OPENTXS_EXPORT virtual std::string ConnectionStatus() const noexcept = 0;

    /** Contact update notifications
     *
     *  A subscribe socket can connect to this endpoint to be notified when
     *  any contact is modified or updated.
     *
     *  Messages bodies consist of one frame.
     *   * The frame contains the contact ID as a serialized string
     *
     *  This endpoint is active for client sessions only.
     */
    OPENTXS_EXPORT virtual std::string ContactUpdate() const noexcept = 0;

    /** Search for a nym in the DHT
     *
     *  A request or dealer socket can connect to this endpoint to trigger
     *  a DHT search for a nym.
     *
     *  Request messages bodies consist of one frame.
     *
     *   * The frame should contains the nym ID being requested as a serialized
     *     string
     *
     *  Reply messages bodies consist of one frame.
     *
     *   * The frame contains a bool with a true value if the nym id provided
     *     was valid or a false value if the nym id was invalid.
     *
     *  This endpoint is active for all session types.
     */
    OPENTXS_EXPORT virtual std::string DhtRequestNym() const noexcept = 0;

    /** Search for a notary in the DHT
     *
     *  A request or dealer socket can connect to this endpoint to trigger
     *  a DHT search for a notary.
     *
     *  Request messages bodies consist of one frame.
     *
     *   * The frame should contains the notary ID being requested as a
     * serialized string
     *
     *  Reply messages bodies consist of one frame.
     *
     *   * The frame contains a bool with a true value if the notary id provided
     *     was valid or a false value if the notary id was invalid.
     *
     *  This endpoint is active for all session types.
     */
    OPENTXS_EXPORT virtual std::string DhtRequestServer() const noexcept = 0;

    /** Search for a unit definition in the DHT
     *
     *  A request or dealer socket can connect to this endpoint to trigger
     *  a DHT search for a unit definition.
     *
     *  Request messages bodies consist of one frame.
     *
     *   * The frame should contains the unit definition ID being requested as a
     * serialized string
     *
     *  Reply messages bodies consist of one frame.
     *
     *   * The frame contains a bool with a true value if the unit definition id
     * provided was valid or a false value if the unit definition id was
     * invalid.
     *
     *  This endpoint is active for all session types.
     */
    OPENTXS_EXPORT virtual std::string DhtRequestUnit() const noexcept = 0;

    /** Search for a nym on known notaries
     *
     *  A push socket can connect to this endpoint to trigger an OTX search for
     * a nym.
     *
     *  Request messages bodies consist of one frame.
     *
     *   * The frame should contains the nym ID being requested as a serialized
     *     string
     *
     *  This endpoint is active for client sessions only.
     */
    OPENTXS_EXPORT virtual std::string FindNym() const noexcept = 0;

    /** Search for a notary contract on known notaries
     *
     *  A push socket can connect to this endpoint to trigger an OTX search for
     * a notary contract.
     *
     *  Request messages bodies consist of one frame.
     *
     *   * The frame should contains the notary ID being requested as a
     * serialized string
     *
     *  This endpoint is active for client sessions only.
     */
    OPENTXS_EXPORT virtual std::string FindServer() const noexcept = 0;

    /** Search for a unit definition on known notaries
     *
     *  A push socket can connect to this endpoint to trigger an OTX search for
     * a unit definition contract.
     *
     *  Request messages bodies consist of one frame.
     *
     *   * The frame should contains the unit definition ID being requested as a
     * serialized string
     *
     *  This endpoint is active for client sessions only.
     */
    OPENTXS_EXPORT virtual std::string FindUnitDefinition() const noexcept = 0;

    /** Communication between blockchain peers and boost::asio context
     *
     */
    virtual std::string InternalBlockchainAsioContext() const noexcept = 0;

    /** Notification of blockchain block filter updates
     *
     */
    virtual std::string InternalBlockchainFilterUpdated(
        const opentxs::blockchain::Type chain) const noexcept = 0;

    /** Load balancing thread pool shared by all blockchains
     *
     */
    virtual std::string InternalBlockchainThreadPool() const noexcept = 0;

    /** Push notification processing
     *
     *  This socket is for use by the Sync and ServerConnection classes only
     */
    virtual std::string InternalProcessPushNotification() const noexcept = 0;

    /** Push notification initiation
     *
     *  This socket is for use by the Server and MessageProcessor classes only
     */
    virtual std::string InternalPushNotification() const noexcept = 0;

    /** Issuer update notifications
     *
     *  A subscribe socket can connect to this endpoint to be notified when
     *  any issuer is modified or updated.
     *
     *  Messages bodies consist of one frame.
     *   * The frame contains the issuer nym ID as a serialized string
     *
     *  This endpoint is active for client sessions only.
     */
    OPENTXS_EXPORT virtual std::string IssuerUpdate() const noexcept = 0;

    /** Nym created notifications
     *
     *  A subscribe socket can connect to this endpoint to be notified when
     *  a new nym is created
     *
     *  Messages bodies consist of one frame.
     *   * The frame contains the nym ID as a serialized string
     *
     *  This endpoint is active for all session types.
     */
    OPENTXS_EXPORT virtual std::string NymCreated() const noexcept = 0;

    /** Nym update notifications
     *
     *  A subscribe socket can connect to this endpoint to be notified when
     *  any nym is modified or updated.
     *
     *  Messages bodies consist of one frame.
     *   * The frame contains the nym ID as a serialized string
     *
     *  This endpoint is active for all session types.
     */
    OPENTXS_EXPORT virtual std::string NymDownload() const noexcept = 0;

    /** Node pairing event notification
     *
     *  A subscribe socket can connect to this endpoint to be notified when
     *  any peer message related to node pairing is received.
     *
     *  Messages bodies consist of one frame.
     *   * The frame contains a serialized proto::PairEvent message
     *
     *  This endpoint is active for client sessions only.
     */
    OPENTXS_EXPORT virtual std::string PairEvent() const noexcept = 0;

    /** Peer reply event notification
     *
     *  A subscribe socket can connect to this endpoint to be notified when
     *  any peer reply is received.
     *
     *  Messages bodies consist of two frame.
     *   * The first frame contains the recipient nym as a serialized string
     *   * The second frame contains a serialized proto::PeerReply message
     *
     *  This endpoint is active for client sessions only.
     */
    OPENTXS_EXPORT virtual std::string PeerReplyUpdate() const noexcept = 0;

    /** Peer request event notification
     *
     *  A subscribe socket can connect to this endpoint to be notified when
     *  any peer request is received.
     *
     *  Messages bodies consist of one frame.
     *   * The first frame contains the recipient nym as a serialized string
     *   * The second frame contains a serialized proto::PeerRequest message
     *
     *  This endpoint is active for client sessions only.
     */
    OPENTXS_EXPORT virtual std::string PeerRequestUpdate() const noexcept = 0;

    /** Pending bailment notification
     *
     *  A subscribe socket can connect to this endpoint to be notified when
     *  a pending bailment peer request has been received.
     *
     *  Messages bodies consist of one frame.
     *   * The frame contains a serialized proto::PeerRequest message
     *
     *  This endpoint is active for client sessions only.
     */
    OPENTXS_EXPORT virtual std::string PendingBailment() const noexcept = 0;

    /** Server reply notification
     *
     *  A subscribe socket can connect to this endpoint to be notified when
     *  any server reply is received.
     *
     *  Messages bodies consist of one frame.
     *   * The frame contains of the message type as a string
     *
     *  This endpoint is active for client sessions only.
     */
    OPENTXS_EXPORT virtual std::string ServerReplyReceived() const noexcept = 0;

    /** Server request notification
     *
     *  A subscribe socket can connect to this endpoint to be notified when
     *  any request message is sent to a notary.
     *
     *  Messages bodies consist of one frame.
     *   * The frame contains of the message type as a string
     *
     *  This endpoint is active for client sessions only.
     */
    OPENTXS_EXPORT virtual std::string ServerRequestSent() const noexcept = 0;

    /** Server contract download notifications
     *
     *  A subscribe socket can connect to this endpoint to be notified when
     *  any new server contract is added to the wallet
     *
     *  Messages bodies consist of one frame.
     *   * The frame contains the server ID as a serialized string
     *
     *  This endpoint is active for all session types.
     */
    OPENTXS_EXPORT virtual std::string ServerUpdate() const noexcept = 0;

    /** Notification of context shutdown
     *
     *  A subscribe socket can connect to this endpoint to be notified when
     *  a session is being shutdown
     *
     *  Messages bodies consist of one empty frame
     *
     *  This endpoint is active for all session types.
     */
    OPENTXS_EXPORT virtual std::string Shutdown() const noexcept = 0;

    /** Background task completion notification
     *
     *  A subscribe socket can connect to this endpoint to be notified when
     *  a background task has concluded.
     *
     *  Resolution is a boolean value that is true if the task executed
     *  successfully, or false in the event of a failure.
     *
     *  Messages bodies consist of two frames.
     *   * The first frame contains the task ID as a serialized string
     *   * The second frame contains the resolution as a bool
     *
     *  This endpoint is active for client sessions only.
     */
    OPENTXS_EXPORT virtual std::string TaskComplete() const noexcept = 0;

    /** Activity thread update notification
     *
     *  A subscribe socket can connect to this endpoint to be notified when
     *  any activity thread for a specified nym receives new activity.
     *
     *  Messages bodies consist of one frame.
     *   * The frame contains the thread ID as a serialized string
     *
     *  This endpoint is active for client sessions only.
     */
    OPENTXS_EXPORT virtual std::string ThreadUpdate(
        const std::string& thread) const noexcept = 0;

    /** UI widget update notification
     *
     *  A subscribe socket can connect to this endpoint to be notified when
     *  any ui widget updates
     *
     *  Messages bodies consist of one frame.
     *   * The frame contains the widget ID as a serialized string
     *
     *  This endpoint is active for client sessions only.
     */
    OPENTXS_EXPORT virtual std::string WidgetUpdate() const noexcept = 0;

    /** Account update notification
     *
     *  A subscribe socket can connect to this endpoint to be notified when
     *  any account receives new activity.
     *
     *  Messages bodies consist of one frame.
     *   * The frame contains the account ID as a serialized string
     *
     *  This endpoint is active for client sessions only.
     */
    OPENTXS_EXPORT virtual std::string WorkflowAccountUpdate() const
        noexcept = 0;

    OPENTXS_EXPORT virtual ~Endpoints() = default;

protected:
    Endpoints() = default;

private:
    Endpoints(const Endpoints&) = delete;
    Endpoints(Endpoints&&) = delete;
    Endpoints& operator=(const Endpoints&) = delete;
    Endpoints& operator=(Endpoints&&) = delete;
};
}  // namespace api
}  // namespace opentxs
#endif
