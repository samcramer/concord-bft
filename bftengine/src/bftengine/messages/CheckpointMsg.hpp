// Concord
//
// Copyright (c) 2018 VMware, Inc. All Rights Reserved.
//
// This product is licensed to you under the Apache 2.0 license (the "License").  You may not use this product except in
// compliance with the Apache 2.0 License.
//
// This product may include a number of subcomponents with separate copyright notices and license terms. Your use of
// these subcomponents is subject to the terms and conditions of the subcomponent's license, as noted in the LICENSE
// file.

#pragma once

#include "MessageBase.hpp"
#include "Digest.hpp"
#include "ReplicaConfig.hpp"

using concord::util::digest::Digest;

namespace bftEngine::impl {

class CheckpointMsg : public MessageBase {
 public:
  typedef std::uint64_t State;

  CheckpointMsg(ReplicaId genReplica,
                SeqNum seqNum,
                State state,
                const Digest& stateDigest,
                const Digest& fullStateDigest,
                bool stateIsStable,
                const concordUtils::SpanContext& spanContext = concordUtils::SpanContext{});

  BFTENGINE_GEN_CONSTRUCT_FROM_BASE_MESSAGE(CheckpointMsg)

  SeqNum seqNumber() const { return b()->seqNum; }

  EpochNum epochNumber() const { return b()->epochNum; }

  State state() const { return b()->state; }

  Digest& digestOfState() const { return b()->stateDigest; }

  Digest& otherDigest() const { return b()->otherDigest; }

  uint16_t idOfGeneratedReplica() const { return b()->genReplicaId; }

  bool isStableState() const { return (b()->flags & 0x1) != 0; }

  void setStateAsStable() { b()->flags |= 0x1; }

  void setEpochNumber(const EpochNum& e) { b()->epochNum = e; }

  void validate(const ReplicasInfo& repInfo) const override;

  bool shouldValidateAsync() const override { return true; }

  void sign();

  void setSenderId(NodeIdType id) { sender_ = id; }
  static bool equivalent(CheckpointMsg* a, CheckpointMsg* b) {
    return (a->seqNumber() == b->seqNumber()) && (a->digestOfState() == b->digestOfState()) &&
           (a->otherDigest() == b->otherDigest()) && (a->state() == b->state());
  }
  inline size_t getHeaderLen() const { return sizeof(Header); }

 protected:
  template <typename MessageT>
  friend size_t sizeOfHeader();

#pragma pack(push, 1)
  struct Header {
    MessageBase::Header header;
    SeqNum seqNum;
    EpochNum epochNum;
    State state;
    Digest stateDigest;
    Digest otherDigest;
    ReplicaId genReplicaId;  // the replica that originally generated this message
    uint8_t flags;           // followed by a signature (by genReplicaId)
  };
#pragma pack(pop)
  static_assert(sizeof(Header) == (6 + 8 + 8 + 8 + DIGEST_SIZE + DIGEST_SIZE + 2 + 1), "Header is 97B");

  Header* b() const { return (Header*)msgBody_; }
};

template <>
inline MsgSize maxMessageSize<CheckpointMsg>() {
  return ReplicaConfig::instance().getmaxExternalMessageSize() + MessageBase::SPAN_CONTEXT_MAX_SIZE;
}

}  // namespace bftEngine::impl
