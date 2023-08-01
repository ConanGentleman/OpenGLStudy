#version 330 core
layout (points) in; //从顶点着色器接收图元值（点）
layout (triangle_strip, max_vertices = 5) out;//几何着色器的图元输出为triangle_strip（三角形），最大的定点数为5

in VS_OUT {
    vec3 color;
} gs_in[];

out vec3 fColor;

void build_house(vec4 position){
    fColor =gs_in[0].color;// 输入只有一个顶点，整个图元的颜色就是该点的颜色
    gl_Position = position + vec4(-0.2, -0.2, 0.0, 0.0);// 基于输入的点的位置 在其vec4(-0.2, -0.2, 0.0, 0.0)的位置上生成一个顶点，且颜色为fColor （左下点）
    EmitVertex();//调用EmitVertex时，gl_Position中的向量会被添加到图元中
    gl_Position = position + vec4( 0.2, -0.2, 0.0, 0.0); // 2:bottom-right
    EmitVertex();
    gl_Position = position + vec4(-0.2,  0.2, 0.0, 0.0); // 3:top-left
    EmitVertex();
    gl_Position = position + vec4( 0.2,  0.2, 0.0, 0.0); // 4:top-right
    EmitVertex();
    gl_Position = position + vec4( 0.0,  0.4, 0.0, 0.0); // 5:top
    fColor = vec3(1.0, 1.0, 1.0);//最上顶点设置为白色
    EmitVertex();
    EndPrimitive();//当EndPrimitive被调用时，所有发射出的(Emitted)顶点都会合成为指定的输出渲染图元
}
void main() {    
    build_house(gl_in[0].gl_Position);
}