#pragma once

namespace Game
{
class Intersection
{
  public:
    static bool CircleRectIntersect(const glm::vec2& circle, float r, const glm::vec4& rect)
    {
        auto circleDistancex = glm::abs(circle.x - rect.x);
        auto circleDistancey = glm::abs(circle.y - rect.y);

        if (circleDistancex > (rect.z / 2 + r) || circleDistancey > (rect.w / 2 + r))
            return false;

        if (circleDistancex <= (rect.z / 2) || circleDistancey <= (rect.w / 2))
            return true;

        auto cornerDistance_sq = glm::pow((circleDistancex - rect.z / 2), 2.0f) + glm::pow((circleDistancey - rect.w / 2), 2.0f);

        return (cornerDistance_sq <= glm::pow(r, 2.0f));
    }

    static bool CircleCircleIntersect(const glm::vec2& c1, float r1, const glm::vec2& c2, float r2)
    {
        return glm::pow(glm::distance(c1, c2), 2.0f) < (r1 + r2);
    }
};
} // namespace Game