/*
 * COPYRIGHT:   See COPYING in the top level directory
 * PROJECT:     ReactOS NDIS library
 * FILE:        ndis/hardware.c
 * PURPOSE:     Hardware related routines
 * PROGRAMMERS: Casper S. Hornstrup (chorns@users.sourceforge.net)
 *              Vizzini (vizzini@plasmic.com)
 * REVISIONS:
 *   CSH 01/08-2000 Created
 *   25 Aug 2003 Vizzini - NDIS4/5 and PnP additions
 *   3  Oct 2003 Vizzini - formatting and minor bugfixes
 *
 * TODO:
 *    - Fix HalGetBusDataByOffset() param 2 in most calls below
 */

#include <roscfg.h>
#include "ndissys.h"


/*
 * @implemented
 */
ULONG
EXPORT
NdisImmediateReadPciSlotInformation(
    IN  NDIS_HANDLE WrapperConfigurationContext,
    IN  ULONG       SlotNumber,
    IN  ULONG       Offset,
    IN  PVOID       Buffer,
    IN  ULONG       Length)
{
  return HalGetBusDataByOffset (PCIConfiguration, 0, SlotNumber, Buffer, Offset, Length);
}


/*
 * @implemented
 */
ULONG 
EXPORT
NdisImmediateWritePciSlotInformation(
    IN  NDIS_HANDLE WrapperConfigurationContext,
    IN  ULONG       SlotNumber,
    IN  ULONG       Offset,
    IN  PVOID       Buffer,
    IN  ULONG       Length)
{
  return HalSetBusDataByOffset (PCIConfiguration, 0, SlotNumber, Buffer, Offset, Length);
}


/*
 * @implemented
 */
NDIS_STATUS
EXPORT
NdisMPciAssignResources(
    IN  NDIS_HANDLE             MiniportHandle,
    IN  ULONG                   SlotNumber,
    OUT PNDIS_RESOURCE_LIST     *AssignedResources)
/*
 * NOTES:
 *     - I think this is fundamentally broken
 */
{
  PCM_RESOURCE_LIST ResourceList;
  NTSTATUS Status;
  PLOGICAL_ADAPTER Adapter = (PLOGICAL_ADAPTER)MiniportHandle;

  ResourceList = NULL;
  Status = HalAssignSlotResources (Adapter->Miniport->RegistryPath,
				   0,
				   Adapter->Miniport->DriverObject,
				   0,
				   PCIBus,
				   Adapter->NdisMiniportBlock.BusNumber,
				   SlotNumber,
				   &ResourceList);
  if (!NT_SUCCESS (Status))
    {
      *AssignedResources = NULL;
      return NDIS_STATUS_FAILURE;
    }

  *AssignedResources = (PNDIS_RESOURCE_LIST)&ResourceList->List[0].PartialResourceList;

  return NDIS_STATUS_SUCCESS;
}


/*
 * @implemented
 */
VOID
EXPORT
NdisMQueryAdapterResources(
    OUT     PNDIS_STATUS        Status,
    IN      NDIS_HANDLE         WrapperConfigurationContext,
    OUT     PNDIS_RESOURCE_LIST ResourceList,
    IN OUT  PUINT               BufferSize)
/*
 * FUNCTION: returns a nic's hardware resources
 * ARGUMENTS:
 *     Status: on return, contains the status of the operation
 *     WrapperConfigurationContext: handle input to MiniportInitialize 
 *     ResourceList: on return, contains the list of resources for the nic
 *     BufferSize: size of ResourceList
 * NOTES:
 *     - Caller must allocate Status and ResourceList
 *     - Must be called at IRQL = PASSIVE_LEVEL;
 */
{
  PNDIS_WRAPPER_CONTEXT WrapperContext = (PNDIS_WRAPPER_CONTEXT)WrapperConfigurationContext;
  PNDIS_MINIPORT_BLOCK MiniportBlock = WrapperContext->PhysicalDeviceObject->DeviceExtension;
  ULONG ResourceListSize;

  PAGED_CODE();
  ASSERT(Status && ResourceList);

  /* FIXME: We can't do anything in this case. It shouldn't really happen. */
  if (MiniportBlock->AllocatedResources == NULL)
    {
      NDIS_DbgPrint(MIN_TRACE, ("Unimplemented!\n"));
      *Status = STATUS_NOT_SUPPORTED;
      return;
    }

  ResourceListSize = FIELD_OFFSET(CM_RESOURCE_LIST, List) +
                     MiniportBlock->AllocatedResources->Count *
                     sizeof(CM_FULL_RESOURCE_DESCRIPTOR);

  if (*BufferSize >= ResourceListSize)
    {
      RtlCopyMemory(ResourceList, MiniportBlock->AllocatedResources, ResourceListSize);
      *BufferSize = ResourceListSize;
      *Status = STATUS_SUCCESS;
    }
  else
    {
      *Status = NDIS_STATUS_RESOURCES;
    }
}


/*
 * @implemented
 */
NDIS_STATUS
EXPORT
NdisQueryMapRegisterCount(
    IN  NDIS_INTERFACE_TYPE BusType,
    OUT PUINT               MapRegisterCount)
/*
 * On X86 (and all other current hardware), map registers aren't real hardware,
 * and there is no real limit to the number that can be allocated.
 * As such, we do what microsoft does on the x86 hals and return as follows
 */
{
  return NDIS_STATUS_NOT_SUPPORTED;
}


/*
 * @unimplemented
 */
VOID
EXPORT
NdisReadEisaSlotInformation(
    OUT PNDIS_STATUS                    Status,
    IN  NDIS_HANDLE                     WrapperConfigurationContext,
    OUT PUINT                           SlotNumber,
    OUT PNDIS_EISA_FUNCTION_INFORMATION EisaData)
{
    UNIMPLEMENTED
}


/*
 * @unimplemented
 */
VOID
EXPORT
NdisReadEisaSlotInformationEx(
    OUT PNDIS_STATUS                    Status,
    IN  NDIS_HANDLE                     WrapperConfigurationContext,
    OUT PUINT                           SlotNumber,
    OUT PNDIS_EISA_FUNCTION_INFORMATION *EisaData,
    OUT PUINT                           NumberOfFunctions)
{
    UNIMPLEMENTED
}


/*
 * @implemented
 */
ULONG
EXPORT
NdisReadPciSlotInformation(
    IN  NDIS_HANDLE NdisAdapterHandle,
    IN  ULONG       SlotNumber,
    IN  ULONG       Offset,
    IN  PVOID       Buffer,
    IN  ULONG       Length)
{
  PLOGICAL_ADAPTER Adapter = (PLOGICAL_ADAPTER)NdisAdapterHandle;
  /* Slot number is ignored since W2K for all NDIS drivers. */
  return HalGetBusDataByOffset (PCIConfiguration, 0,
                                Adapter->NdisMiniportBlock.SlotNumber,
                                Buffer, Offset, Length);
}


/*
 * @implemented
 */
ULONG
EXPORT
NdisWritePciSlotInformation(
    IN  NDIS_HANDLE NdisAdapterHandle,
    IN  ULONG       SlotNumber,
    IN  ULONG       Offset,
    IN  PVOID       Buffer,
    IN  ULONG       Length)
{
  PLOGICAL_ADAPTER Adapter = (PLOGICAL_ADAPTER)NdisAdapterHandle;
  /* Slot number is ignored since W2K for all NDIS drivers. */
  return HalSetBusDataByOffset (PCIConfiguration, 0, 
                                Adapter->NdisMiniportBlock.SlotNumber,
                                Buffer, Offset, Length);
}

/* EOF */

