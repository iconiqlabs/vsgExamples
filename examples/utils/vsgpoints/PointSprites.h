#include <vsg/commands/Draw.h>
#include <vsg/core/Array.h>
#include <vsg/nodes/StateGroup.h>
#include <vsg/commands/BindVertexBuffers.h>

namespace vsg
{
    vsg::ref_ptr<vsg::Data> createParticleImage(uint32_t dim);

    vsg::ref_ptr<vsg::StateGroup> createPointSpriteStateGroup(vsg::ref_ptr<const vsg::Options> options);

    class PointSprites : public vsg::Inherit<vsg::Group, PointSprites>
    {
    public:
        PointSprites(uint32_t maxNumParticles = 1024, vsg::ref_ptr<const vsg::Options> options = {});
        PointSprites(const vsg::DataList& arrays, vsg::ref_ptr<const vsg::Options> options = {});

        void set(size_t i, const vsg::vec3& vertex, const vsg::vec3& normal = {0.0f, 0.0f, 1.0}, const vsg::vec4& color = vsg::vec4(1.0, 1.0, 1.0, 1.0));
        void set(size_t i, const vsg::vec3& vertex, const vsg::vec3& normal, const vsg::ubvec4& color);

        vsg::ref_ptr<vsg::vec3Array> vertices;
        vsg::ref_ptr<vsg::vec3Array> normals;
        vsg::ref_ptr<vsg::ubvec4Array> colors;
        vsg::ref_ptr<vsg::StateGroup> stateGroup;
        vsg::ref_ptr<vsg::BindVertexBuffers> bindVertexBuffers;
        vsg::ref_ptr<vsg::Draw> draw;
    };
    VSG_type_name(vsg::PointSprites);

}
