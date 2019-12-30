// Copyright 2013 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef V8_COMPILER_NODE_PROPERTIES_H_
#define V8_COMPILER_NODE_PROPERTIES_H_

#include "src/compiler/node.h"
#include "src/compiler/types.h"
#include "src/globals.h"
#include "src/objects/map.h"
#include "src/zone/zone-handle-set.h"

namespace v8 {
namespace internal {
namespace compiler {

class Graph;
class Operator;
class CommonOperatorBuilder;

// A facade that simplifies access to the different kinds of inputs to a node.
class V8_EXPORT_PRIVATE NodeProperties final {
 public:
  // ---------------------------------------------------------------------------
  // Input layout.
  // Inputs are always arranged in order as follows:
  //     0 [ values, context, frame state, effects, control ] node->InputCount()

  static int FirstValueIndex(Node* node) { return 0; }
  static int FirstContextIndex(Node* node) { return PastValueIndex(node); }
  static int FirstFrameStateIndex(Node* node) { return PastContextIndex(node); }
  static int FirstEffectIndex(Node* node) { return PastFrameStateIndex(node); }
  static int FirstControlIndex(Node* node) { return PastEffectIndex(node); }
  static int PastValueIndex(Node* node);
  static int PastContextIndex(Node* node);
  static int PastFrameStateIndex(Node* node);
  static int PastEffectIndex(Node* node);
  static int PastControlIndex(Node* node);


  // ---------------------------------------------------------------------------
  // Input accessors.

  static Node* GetValueInput(Node* node, int index);
  static Node* GetContextInput(Node* node);
  static Node* GetFrameStateInput(Node* node);
  static Node* GetEffectInput(Node* node, int index = 0);
  static Node* GetControlInput(Node* node, int index = 0);


  // ---------------------------------------------------------------------------
  // Edge kinds.

  static bool IsValueEdge(Edge edge);
  static bool IsContextEdge(Edge edge);
  static bool IsFrameStateEdge(Edge edge);
  static bool IsEffectEdge(Edge edge);
  static bool IsControlEdge(Edge edge);


  // ---------------------------------------------------------------------------
  // Miscellaneous predicates.

  static bool IsCommon(Node* node) {
    return IrOpcode::IsCommonOpcode(node->opcode());
  }
  static bool IsControl(Node* node) {
    return IrOpcode::IsControlOpcode(node->opcode());
  }
  static bool IsConstant(Node* node) {
    return IrOpcode::IsConstantOpcode(node->opcode());
  }
  static bool IsPhi(Node* node) {
    return IrOpcode::IsPhiOpcode(node->opcode());
  }

  // Determines whether exceptions thrown by the given node are handled locally
  // within the graph (i.e. an IfException projection is present). Optionally
  // the present IfException projection is returned via {out_exception}.
  static bool IsExceptionalCall(Node* node, Node** out_exception = nullptr);

  // Returns the node producing the successful control output of {node}. This is
  // the IfSuccess projection of {node} if present and {node} itself otherwise.
  static Node* FindSuccessfulControlProjection(Node* node);

  // ---------------------------------------------------------------------------
  // Miscellaneous mutators.

  static void ReplaceValueInput(Node* node, Node* value, int index);
  static void ReplaceContextInput(Node* node, Node* context);
  static void ReplaceControlInput(Node* node, Node* control, int index = 0);
  static void ReplaceEffectInput(Node* node, Node* effect, int index = 0);
  static void ReplaceFrameStateInput(Node* node, Node* frame_state);
  static void RemoveNonValueInputs(Node* node);
  static void RemoveValueInputs(Node* node);

  // Replaces all value inputs of {node} with the single input {value}.
  static void ReplaceValueInputs(Node* node, Node* value);

  // Merge the control node {node} into the end of the graph, introducing a
  // merge node or expanding an existing merge node if necessary.
  static void MergeControlToEnd(Graph* graph, CommonOperatorBuilder* common,
                                Node* node);

  // Replace all uses of {node} with the given replacement nodes. All occurring
  // use kinds need to be replaced, {nullptr} is only valid if a use kind is
  // guaranteed not to exist.
  static void ReplaceUses(Node* node, Node* value, Node* effect = nullptr,
                          Node* success = nullptr, Node* exception = nullptr);

  // Safe wrapper to mutate the operator of a node. Checks that the node is
  // currently in a state that satisfies constraints of the new operator.
  static void ChangeOp(Node* node, const Operator* new_op);

  // ---------------------------------------------------------------------------
  // Miscellaneous utilities.

  // Find the last frame state that is effect-wise before the given node. This
  // assumes a linear effect-chain up to a {CheckPoint} node in the graph.
  static Node* FindFrameStateBefore(Node* node);

  // Collect the output-value projection for the given output index.
  static Node* FindProjection(Node* node, size_t projection_index);

  // Collect the value projections from a node.
  static void CollectValueProjections(Node* node, Node** proj, size_t count);

  // Collect the branch-related projections from a node, such as IfTrue,
  // IfFalse, IfSuccess, IfException, IfValue and IfDefault.
  //  - Branch: [ IfTrue, IfFalse ]
  //  - Call  : [ IfSuccess, IfException ]
  //  - Switch: [ IfValue, ..., IfDefault ]
  static void CollectControlProjections(Node* node, Node** proj, size_t count);

  // Checks if two nodes are the same, looking past {CheckHeapObject}.
  static bool IsSame(Node* a, Node* b);

  // Check if two nodes have equal operators and reference-equal inputs. Used
  // for value numbering/hash-consing.
  static bool Equals(Node* a, Node* b);
  // A corresponding hash function.
  static size_t HashCode(Node* node);

  // Walks up the {effect} chain to find a witness that provides map
  // information about the {receiver}. Can look through potentially
  // side effecting nodes.
  enum InferReceiverMapsResult {
    kNoReceiverMaps,         // No receiver maps inferred.
    kReliableReceiverMaps,   // Receiver maps can be trusted.
    kUnreliableReceiverMaps  // Receiver maps might have changed (side-effect),
                             // but instance type is reliable.
  };
  static InferReceiverMapsResult InferReceiverMaps(
      JSHeapBroker* broker, Node* receiver, Node* effect,
      ZoneHandleSet<Map>* maps_return);

  static bool HasInstanceTypeWitness(JSHeapBroker* broker, Node* receiver,
                                     Node* effect, InstanceType instance_type);

  // Walks up the {effect} chain to check that there's no observable side-effect
  // between the {effect} and it's {dominator}. Aborts the walk if there's join
  // in the effect chain.
  static bool NoObservableSideEffectBetween(Node* effect, Node* dominator);

  // Returns true if the {receiver} can be a primitive value (i.e. is not
  // definitely a JavaScript object); might walk up the {effect} chain to
  // find map checks on {receiver}.
  static bool CanBePrimitive(JSHeapBroker* broker, Node* receiver,
                             Node* effect);

  // Returns true if the {receiver} can be null or undefined. Might walk
  // up the {effect} chain to find map checks for {receiver}.
  static bool CanBeNullOrUndefined(JSHeapBroker* broker, Node* receiver,
                                   Node* effect);

  // ---------------------------------------------------------------------------
  // Context.

  // Walk up the context chain from the given {node} until we reduce the {depth}
  // to 0 or hit a node that does not extend the context chain ({depth} will be
  // updated accordingly).
  static Node* GetOuterContext(Node* node, size_t* depth);

  // ---------------------------------------------------------------------------
  // Type.

  static bool IsTyped(Node* node) { return !node->type().IsInvalid(); }
  static Type GetType(Node* node) {
    DCHECK(IsTyped(node));
    return node->type();
  }
  static Type GetTypeOrAny(Node* node);
  static void SetType(Node* node, Type type) {
    DCHECK(!type.IsInvalid());
    node->set_type(type);
  }
  static void RemoveType(Node* node) { node->set_type(Type::Invalid()); }
  static bool AllValueInputsAreTyped(Node* node);

 private:
  static inline bool IsInputRange(Edge edge, int first, int count);
};

}  // namespace compiler
}  // namespace internal
}  // namespace v8

#endif  // V8_COMPILER_NODE_PROPERTIES_H_
