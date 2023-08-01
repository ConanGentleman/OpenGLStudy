#version 330 core
layout (points) in; //�Ӷ�����ɫ������ͼԪֵ���㣩
layout (triangle_strip, max_vertices = 5) out;//������ɫ����ͼԪ���Ϊtriangle_strip�������Σ������Ķ�����Ϊ5

in VS_OUT {
    vec3 color;
} gs_in[];

out vec3 fColor;

void build_house(vec4 position){
    fColor =gs_in[0].color;// ����ֻ��һ�����㣬����ͼԪ����ɫ���Ǹõ����ɫ
    gl_Position = position + vec4(-0.2, -0.2, 0.0, 0.0);// ��������ĵ��λ�� ����vec4(-0.2, -0.2, 0.0, 0.0)��λ��������һ�����㣬����ɫΪfColor �����µ㣩
    EmitVertex();//����EmitVertexʱ��gl_Position�е������ᱻ��ӵ�ͼԪ��
    gl_Position = position + vec4( 0.2, -0.2, 0.0, 0.0); // 2:bottom-right
    EmitVertex();
    gl_Position = position + vec4(-0.2,  0.2, 0.0, 0.0); // 3:top-left
    EmitVertex();
    gl_Position = position + vec4( 0.2,  0.2, 0.0, 0.0); // 4:top-right
    EmitVertex();
    gl_Position = position + vec4( 0.0,  0.4, 0.0, 0.0); // 5:top
    fColor = vec3(1.0, 1.0, 1.0);//���϶�������Ϊ��ɫ
    EmitVertex();
    EndPrimitive();//��EndPrimitive������ʱ�����з������(Emitted)���㶼��ϳ�Ϊָ���������ȾͼԪ
}
void main() {    
    build_house(gl_in[0].gl_Position);
}