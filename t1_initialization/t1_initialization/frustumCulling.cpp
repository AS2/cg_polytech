#include "frustumCulling.h"

void FrustumCulling::ConstructFrustum(XMMATRIX viewMatrix, XMMATRIX projectionMatrix) {
  // Convert the projection matrix into a 4x4 float type.
  XMFLOAT4X4 pMatrix;
  XMStoreFloat4x4(&pMatrix, projectionMatrix);

  // Calculate the minimum Z distance in the frustum.
  float zMinimum = -pMatrix._43 / pMatrix._33;
  float r = screenDepth / (screenDepth - zMinimum);

  // Load the updated values back into the projection matrix.
  pMatrix._33 = r;
  pMatrix._43 = -r * zMinimum;
  projectionMatrix = XMLoadFloat4x4(&pMatrix);

  // Create the frustum matrix from the view matrix and updated projection matrix.
  XMMATRIX finalMatrix = XMMatrixMultiply(viewMatrix, projectionMatrix);

  // Convert the final matrix into a 4x4 float type.
  XMFLOAT4X4 matrix;
  XMStoreFloat4x4(&matrix, finalMatrix);

  // Calculate near plane of frustum.
  planes[0].x = matrix._14 + matrix._13;
  planes[0].y = matrix._24 + matrix._23;
  planes[0].z = matrix._34 + matrix._33;
  planes[0].w = matrix._44 + matrix._43;

  // Normalize the near plane.
  float length = sqrtf((planes[0].x * planes[0].x) + (planes[0].y * planes[0].y) + (planes[0].z * planes[0].z));
  planes[0].x /= length;
  planes[0].y /= length;
  planes[0].z /= length;
  planes[0].w /= length;

  // Calculate far plane of frustum.
  planes[1].x = matrix._14 - matrix._13;
  planes[1].y = matrix._24 - matrix._23;
  planes[1].z = matrix._34 - matrix._33;
  planes[1].w = matrix._44 - matrix._43;

  // Normalize the far plane.
  length = sqrtf((planes[1].x * planes[1].x) + (planes[1].y * planes[1].y) + (planes[1].z * planes[1].z));
  planes[1].x /= length;
  planes[1].y /= length;
  planes[1].z /= length;
  planes[1].w /= length;
  
  // Calculate left plane of frustum.
  planes[2].x = matrix._14 + matrix._11;
  planes[2].y = matrix._24 + matrix._21;
  planes[2].z = matrix._34 + matrix._31;
  planes[2].w = matrix._44 + matrix._41;

  // Normalize the left plane.
  length = sqrtf((planes[2].x * planes[2].x) + (planes[2].y * planes[2].y) + (planes[2].z * planes[2].z));
  planes[2].x /= length;
  planes[2].y /= length;
  planes[2].z /= length;
  planes[2].w /= length;

  // Calculate right plane of frustum.
  planes[3].x = matrix._14 - matrix._11;
  planes[3].y = matrix._24 - matrix._21;
  planes[3].z = matrix._34 - matrix._31;
  planes[3].w = matrix._44 - matrix._41;

  // Normalize the right plane.
  length = sqrtf((planes[3].x * planes[3].x) + (planes[3].y * planes[3].y) + (planes[3].z * planes[3].z));
  planes[3].x /= length;
  planes[3].y /= length;
  planes[3].z /= length;
  planes[3].w /= length;

  // Calculate top plane of frustum.
  planes[4].x = matrix._14 - matrix._12;
  planes[4].y = matrix._24 - matrix._22;
  planes[4].z = matrix._34 - matrix._32;
  planes[4].w = matrix._44 - matrix._42;

  // Normalize the top plane.
  length = sqrtf((planes[4].x * planes[4].x) + (planes[4].y * planes[4].y) + (planes[4].z * planes[4].z));
  planes[4].x /= length;
  planes[4].y /= length;
  planes[4].z /= length;
  planes[4].w /= length;

  // Calculate bottom plane of frustum.
  planes[5].x = matrix._14 + matrix._12;
  planes[5].y = matrix._24 + matrix._22;
  planes[5].z = matrix._34 + matrix._32;
  planes[5].w = matrix._44 + matrix._42;

  // Normalize the bottom plane.
  length = sqrtf((planes[5].x * planes[5].x) + (planes[5].y * planes[5].y) + (planes[5].z * planes[5].z));
  planes[5].x /= length;
  planes[5].y /= length;
  planes[5].z /= length;
  planes[5].w /= length;
}

bool FrustumCulling::CheckRectangle(XMFLOAT4 bbMin, XMFLOAT4 bbMax) {
  for (int i = 0; i < 6; i++) {
    if (((planes[i].x * bbMin.x) + (planes[i].y * bbMin.y) + (planes[i].z * bbMin.z) + (planes[i].w * 1.0f)) >= 0.0f ||
      ((planes[i].x * bbMax.x) + (planes[i].y * bbMin.y) + (planes[i].z * bbMin.z) + (planes[i].w * 1.0f)) >= 0.0f ||
      ((planes[i].x * bbMin.x) + (planes[i].y * bbMax.y) + (planes[i].z * bbMin.z) + (planes[i].w * 1.0f)) >= 0.0f ||
      ((planes[i].x * bbMax.x) + (planes[i].y * bbMax.y) + (planes[i].z * bbMin.z) + (planes[i].w * 1.0f)) >= 0.0f ||
      ((planes[i].x * bbMin.x) + (planes[i].y * bbMin.y) + (planes[i].z * bbMax.z) + (planes[i].w * 1.0f)) >= 0.0f ||
      ((planes[i].x * bbMax.x) + (planes[i].y * bbMin.y) + (planes[i].z * bbMax.z) + (planes[i].w * 1.0f)) >= 0.0f ||
      ((planes[i].x * bbMin.x) + (planes[i].y * bbMax.y) + (planes[i].z * bbMax.z) + (planes[i].w * 1.0f)) >= 0.0f ||
      ((planes[i].x * bbMax.x) + (planes[i].y * bbMax.y) + (planes[i].z * bbMax.z) + (planes[i].w * 1.0f)) >= 0.0f)
      continue;
    else
      return false;
  }

  return true;
}
