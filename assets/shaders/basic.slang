static const let positions: float2[] = { float2(0.0, -0.5), float2(0.5, 0.5), float2(-0.5, 0.5) };

static const let colors : float3[] = { float3(1.0, 0.0, 0.0), float3(0.0, 1.0, 0.0), float3(0.0, 0.0, 1.0) };

struct VertexOutput {
    var color: float3;
    var sv_position: float4 : SV_Position;
};

[shader("vertex")]
func vertex(vertex_id: uint: SV_VertexID) -> VertexOutput {
    var output = VertexOutput();
    output.sv_position = float4(positions[vertex_id], 0.0, 1.0);
    output.color = colors[vertex_id];
    return output;
}

struct FragmentOutput {
    var color: float4 : SV_Target;
};

[shader("fragment")]
func fragment(vert: VertexOutput) -> FragmentOutput {
    var frag = FragmentOutput();
    let color = vert.color;
    frag.color = float4(color, 1);
    return frag;
}
