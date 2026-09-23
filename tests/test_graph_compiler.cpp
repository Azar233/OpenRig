#include "test_support.h"

#include "openrig/core/graph/GraphCompiler.h"
#include "openrig/dsp/drive/SoftClipNode.h"
#include "openrig/dsp/utility/GainNode.h"

#include <memory>

namespace
{
std::unique_ptr<openrig::AudioNode> makeNullNode()
{
    return {};
}

openrig::graph::NodeRegistry makeRegistry()
{
    openrig::graph::NodeRegistry registry;
    static_cast<void>(registry.registerNode("gain", &openrig::dsp::makeGainNode));
    static_cast<void>(registry.registerNode("clip", &openrig::dsp::makeSoftClipNode));
    static_cast<void>(registry.registerNode("null", &makeNullNode));
    return registry;
}

void expectError(
    TestSuite& tests,
    const openrig::graph::GraphDescription& description,
    const openrig::graph::GraphCompileErrorCode expected,
    const std::string_view message)
{
    const openrig::graph::GraphCompiler compiler;
    const auto result = compiler.compile(description, makeRegistry(), {48'000.0, 128, 1, 1});
    tests.check(!result && result.code == expected && !result.error.empty(), message);
}
} // namespace

int main()
{
    using openrig::graph::GraphCompileErrorCode;
    using openrig::graph::GraphDescription;
    TestSuite tests;
    const openrig::graph::GraphCompiler compiler;

    const GraphDescription valid{
        .nodes = {{1, "gain", false, {}}, {2, "clip", false, {}}},
        .connections = {{1, 2}},
    };
    const auto validResult = compiler.compile(valid, makeRegistry(), {48'000.0, 128, 1, 1});
    tests.check(validResult && validResult.code == GraphCompileErrorCode::None, "Linear graph compiles");

    expectError(tests, {{{0, "gain", false, {}}}, {}}, GraphCompileErrorCode::ReservedNodeId, "Reserved id has a stable error code");
    expectError(tests, {{{1, "gain", false, {}}, {1, "clip", false, {}}}, {}}, GraphCompileErrorCode::DuplicateNodeId, "Duplicate id has a stable error code");
    expectError(tests, {{{1, "missing", false, {}}}, {}}, GraphCompileErrorCode::UnregisteredNodeType, "Unknown type has a stable error code");
    expectError(tests, {{{1, "gain", false, {}}}, {{1, 2}}}, GraphCompileErrorCode::MissingConnectionEndpoint, "Missing endpoint has a stable error code");
    expectError(tests, {{{1, "gain", false, {}}, {2, "clip", false, {}}}, {{1, 2}, {1, 2}}}, GraphCompileErrorCode::DuplicateConnection, "Duplicate edge has a stable error code");
    expectError(tests, {{{1, "gain", false, {}}}, {{1, 1}}}, GraphCompileErrorCode::SelfConnection, "Self edge has a stable error code");
    expectError(tests, {{{1, "gain", false, {}}, {2, "gain", false, {}}, {3, "gain", false, {}}}, {{1, 2}, {1, 3}}}, GraphCompileErrorCode::BranchingUnsupported, "Branching has a stable error code");
    expectError(tests, {{{1, "gain", false, {}}, {2, "gain", false, {}}, {3, "gain", false, {}}}, {{1, 3}, {2, 3}}}, GraphCompileErrorCode::MergingUnsupported, "Merging has a stable error code");
    expectError(tests, {{{1, "gain", false, {}}, {2, "gain", false, {}}}, {{1, 2}, {2, 1}}}, GraphCompileErrorCode::Cycle, "Cycle has a stable error code");
    expectError(tests, {{{1, "gain", false, {}}, {2, "gain", false, {}}, {3, "gain", false, {}}}, {{1, 2}}}, GraphCompileErrorCode::DisconnectedGraph, "Disconnected graph has a stable error code");
    expectError(tests, {{{1, "gain", false, {{"not-a-parameter", 1.0F}}}}, {}}, GraphCompileErrorCode::UnknownParameter, "Unknown persisted parameter has a stable error code");
    expectError(tests, {{{1, "null", false, {}}}, {}}, GraphCompileErrorCode::NodeFactoryFailed, "Null factory result has a stable error code");

    const auto invalidSpec = compiler.compile(valid, makeRegistry(), {0.0, 0, 1, 1});
    tests.check(!invalidSpec && invalidSpec.code == GraphCompileErrorCode::InvalidPrepareSpec, "Invalid prepare spec is rejected");
    const auto invalidLayout = compiler.compile(valid, makeRegistry(), {48'000.0, 128, 2, 2});
    tests.check(!invalidLayout && invalidLayout.code == GraphCompileErrorCode::UnsupportedChannelLayout, "Unsupported channel layout is rejected");

    return tests.finish("graph compiler tests");
}
