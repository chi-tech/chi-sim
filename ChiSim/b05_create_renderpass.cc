#include "chi_sim.h"

//###################################################################
/** Create render pass. */
void ChiSim::CreateRenderPass()
{
  VkAttachmentDescription depthAttachment = {};
  depthAttachment.format = FindDepthFormat();
  depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkAttachmentReference depthAttachmentRef = {};
  depthAttachmentRef.attachment = 1;
  depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkAttachmentDescription colorAttachment = {};
  colorAttachment.format         = m_swap_chain_image_format;
  colorAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
  colorAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference colorAttachmentRef = {};
  colorAttachmentRef.attachment = 0;
  colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription sub_pass = {};
  sub_pass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  sub_pass.colorAttachmentCount = 1;
  sub_pass.pColorAttachments = &colorAttachmentRef;
  sub_pass.pDepthStencilAttachment = &depthAttachmentRef;

  VkSubpassDependency dependency = {};
  dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass    = 0;
  dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.srcAccessMask = 0;
  dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  std::array<VkAttachmentDescription, 2> attachments =
    {colorAttachment, depthAttachment};
  VkRenderPassCreateInfo renderPassInfo = {};
  renderPassInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = attachments.size();
  renderPassInfo.pAttachments    = attachments.data();
  renderPassInfo.subpassCount    = 1;
  renderPassInfo.pSubpasses      = &sub_pass;
  renderPassInfo.dependencyCount = 1;
  renderPassInfo.pDependencies   = &dependency;

  if (vkCreateRenderPass(m_device,
                         &renderPassInfo,
                         nullptr,
                         &m_render_pass) != VK_SUCCESS)
    throw std::runtime_error("failed to create render pass!");
}