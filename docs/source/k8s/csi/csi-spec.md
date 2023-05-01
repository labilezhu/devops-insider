# CSI Spec

> [https://github.com/container-storage-interface/spec/blob/master/spec.md](https://github.com/container-storage-interface/spec/blob/master/spec.md)



## Terminology

| Term              | Definition                                                   |
| ----------------- | ------------------------------------------------------------ |
| Volume            | A unit of storage that will be made available inside of a CO-managed container, via the CSI. |
| Block Volume      | A volume that will appear as a block device inside the container. |
| Mounted Volume    | A volume that will be mounted using the specified file system and appear as a directory inside the container. |
| CO                | 可以认为就是 k8s 了。Container Orchestration system, communicates with Plugins using CSI service RPCs. |
| SP                | Storage Provider, the vendor of a CSI plugin implementation. |
| RPC               | [Remote Procedure Call](https://en.wikipedia.org/wiki/Remote_procedure_call). |
| Node              | A host where the user workload will be running, uniquely identifiable from the perspective of a Plugin by a node ID. |
| Plugin            | Aka “plugin implementation”, a gRPC endpoint that implements the CSI Services. |
| Plugin Supervisor | Process that governs the lifecycle of a Plugin, MAY be the CO. |
| Workload          | 即系 k8s pod。The atomic unit of "work" scheduled by a CO. This MAY be a container or a collection of containers. |



## Solution Overview

This specification defines an interface along with the minimum operational and packaging recommendations for a storage provider (SP) to implement a CSI compatible plugin. The interface declares the RPCs that a plugin MUST expose: this is the **primary focus** of the CSI specification. Any operational and packaging recommendations offer additional guidance to promote cross-CO compatibility.

### Architecture

The primary focus of this specification is on the **protocol** between a CO and a Plugin. It SHOULD be possible to ship cross-CO compatible Plugins for a variety of deployment architectures. A CO SHOULD be equipped to handle both `centralized` and `headless plugins`, as well as `split-component` and `unified plugins`. Several of these possibilities are illustrated in the following figures.



#### Centralized - 中心化架构

```
                             CO "Master" Host
+-------------------------------------------+
|                                           |
|  +------------+           +------------+  |
|  |     CO     |   gRPC    | Controller |  |
|  |            +----------->   Plugin   |  |
|  +------------+           +------------+  |
|                                           |
+-------------------------------------------+

                            CO "Node" Host(s)
+-------------------------------------------+
|                                           |
|  +------------+           +------------+  |
|  |     CO     |   gRPC    |    Node    |  |
|  |            +----------->   Plugin   |  |
|  +------------+           +------------+  |
|                                           |
+-------------------------------------------+
```

Figure 1: The Plugin runs on all nodes in the cluster: a centralized `Controller Plugin` is available on the CO master host and the `Node Plugin` is available on all of the CO Nodes.



#### Headless - 去中心架构

只在 worker node 上运行

```
                            CO "Node" Host(s)
+-------------------------------------------+
|                                           |
|  +------------+           +------------+  |
|  |     CO     |   gRPC    | Controller |  |
|  |            +--+-------->   Plugin   |  |
|  +------------+  |        +------------+  |
|                  |                        |
|                  |                        |
|                  |        +------------+  |
|                  |        |    Node    |  |
|                  +-------->   Plugin   |  |
|                           +------------+  |
|                                           |
+-------------------------------------------+
```

Figure 2: Headless Plugin deployment, only the `CO Node` hosts run Plugins. Separate, split-component（分体式） Plugins supply the `Controller Service` and the` Node Service` respectively.





```
                            CO "Node" Host(s)
+-------------------------------------------+
|                                           |
|  +------------+           +------------+  |
|  |     CO     |   gRPC    | Controller |  |
|  |            +----------->    Node    |  |
|  +------------+           |   Plugin   |  |
|                           +------------+  |
|                                           |
+-------------------------------------------+
```

Figure 3: Headless Plugin deployment, only the CO Node hosts run Plugins. A unified Plugin component supplies both the Controller Service and Node Service.





```
                            CO "Node" Host(s)
+-------------------------------------------+
|                                           |
|  +------------+           +------------+  |
|  |     CO     |   gRPC    |    Node    |  |
|  |            +----------->   Plugin   |  |
|  +------------+           +------------+  |
|                                           |
+-------------------------------------------+
```

Figure 4: Headless Plugin deployment, only the CO Node hosts run Plugins. A Node-only Plugin component supplies only the Node Service. Its `GetPluginCapabilities` RPC does not report the `CONTROLLER_SERVICE` capability.



## Volume Lifecycle

### 动态创建 Volume

```
   CreateVolume +------------+ DeleteVolume
 +------------->|  CREATED   +--------------+
 |              +---+----^---+              |
 |       Controller |    | Controller       v
+++         Publish |    | Unpublish       +++
|X|          Volume |    | Volume          | |
+-+             +---v----+---+             +-+
                | NODE_READY |
                +---+----^---+
               Node |    | Node
            Publish |    | Unpublish
             Volume |    | Volume
                +---v----+---+
                | PUBLISHED  |
                +------------+
```

Figure 5: The lifecycle of a `dynamically provisioned volume`, from creation to destruction.



```
   CreateVolume +------------+ DeleteVolume
 +------------->|  CREATED   +--------------+
 |              +---+----^---+              |
 |       Controller |    | Controller       v
+++         Publish |    | Unpublish       +++
|X|          Volume |    | Volume          | |
+-+             +---v----+---+             +-+
                | NODE_READY |
                +---+----^---+
               Node |    | Node
              Stage |    | Unstage
             Volume |    | Volume
                +---v----+---+
                |  VOL_READY |
                +---+----^---+
               Node |    | Node
            Publish |    | Unpublish
             Volume |    | Volume
                +---v----+---+
                | PUBLISHED  |
                +------------+
```

Figure 6: The lifecycle of a dynamically provisioned volume, from creation to destruction, when the `Node Plugin` advertises the `STAGE_UNSTAGE_VOLUME capability`.



### 预先提供 Volume



```
    Controller                  Controller
       Publish                  Unpublish
        Volume  +------------+  Volume
 +------------->+ NODE_READY +--------------+
 |              +---+----^---+              |
 |             Node |    | Node             v
+++         Publish |    | Unpublish       +++
|X| <-+      Volume |    | Volume          | |
+++   |         +---v----+---+             +-+
 |    |         | PUBLISHED  |
 |    |         +------------+
 +----+
   Validate
   Volume
   Capabilities
```

Figure 7: The lifecycle of a pre-provisioned volume that requires controller to publish to a node (`ControllerPublishVolume`) prior to publishing on the node (`NodePublishVolume`).



```
       +-+  +-+
       |X|  | |
       +++  +^+
        |    |
   Node |    | Node
Publish |    | Unpublish
 Volume |    | Volume
    +---v----+---+
    | PUBLISHED  |
    +------------+
```

Figure 8: Plugins MAY forego other lifecycle steps by contraindicating them via the capabilities API. Interactions with the volumes of such plugins is reduced to `NodePublishVolume` and `NodeUnpublishVolume`
calls.

<hr />

The above diagrams illustrate a general expectation with respect to how a `CO` MAY manage the lifecycle of a volume via the API presented in this specification. Plugins SHOULD expose all RPCs for an interface: Controller plugins SHOULD implement all RPCs for the `Controller` service. Unsupported RPCs SHOULD return an appropriate error code that indicates such (e.g. `CALL_NOT_IMPLEMENTED`). The full list of plugin capabilities is documented in the `ControllerGetCapabilities` and `NodeGetCapabilities` RPCs.



## API

There are three sets of RPCs:

- **Identity Service**: Both the Node Plugin and the Controller Plugin MUST implement this sets of RPCs.
- **Controller Service**: The Controller Plugin MUST implement this sets of RPCs.
- **Node Service**: The Node Plugin MUST implement this sets of RPCs.

```c
service Identity {
  rpc GetPluginInfo(GetPluginInfoRequest)
    returns (GetPluginInfoResponse) {}

  rpc GetPluginCapabilities(GetPluginCapabilitiesRequest)
    returns (GetPluginCapabilitiesResponse) {}

  rpc Probe (ProbeRequest)
    returns (ProbeResponse) {}
}

service Controller {
  rpc CreateVolume (CreateVolumeRequest)
    returns (CreateVolumeResponse) {}

  rpc DeleteVolume (DeleteVolumeRequest)
    returns (DeleteVolumeResponse) {}

  rpc ControllerPublishVolume (ControllerPublishVolumeRequest)
    returns (ControllerPublishVolumeResponse) {}

  rpc ControllerUnpublishVolume (ControllerUnpublishVolumeRequest)
    returns (ControllerUnpublishVolumeResponse) {}

  rpc ValidateVolumeCapabilities (ValidateVolumeCapabilitiesRequest)
    returns (ValidateVolumeCapabilitiesResponse) {}

  rpc ListVolumes (ListVolumesRequest)
    returns (ListVolumesResponse) {}

  rpc GetCapacity (GetCapacityRequest)
    returns (GetCapacityResponse) {}

  rpc ControllerGetCapabilities (ControllerGetCapabilitiesRequest)
    returns (ControllerGetCapabilitiesResponse) {}

  rpc CreateSnapshot (CreateSnapshotRequest)
    returns (CreateSnapshotResponse) {}

  rpc DeleteSnapshot (DeleteSnapshotRequest)
    returns (DeleteSnapshotResponse) {}

  rpc ListSnapshots (ListSnapshotsRequest)
    returns (ListSnapshotsResponse) {}

  rpc ControllerExpandVolume (ControllerExpandVolumeRequest)
    returns (ControllerExpandVolumeResponse) {}

  rpc ControllerGetVolume (ControllerGetVolumeRequest)
    returns (ControllerGetVolumeResponse) {
        option (alpha_method) = true;
    }
}

service Node {
  rpc NodeStageVolume (NodeStageVolumeRequest)
    returns (NodeStageVolumeResponse) {}

  rpc NodeUnstageVolume (NodeUnstageVolumeRequest)
    returns (NodeUnstageVolumeResponse) {}

  rpc NodePublishVolume (NodePublishVolumeRequest)
    returns (NodePublishVolumeResponse) {}

  rpc NodeUnpublishVolume (NodeUnpublishVolumeRequest)
    returns (NodeUnpublishVolumeResponse) {}

  rpc NodeGetVolumeStats (NodeGetVolumeStatsRequest)
    returns (NodeGetVolumeStatsResponse) {}


  rpc NodeExpandVolume(NodeExpandVolumeRequest)
    returns (NodeExpandVolumeResponse) {}


  rpc NodeGetCapabilities (NodeGetCapabilitiesRequest)
    returns (NodeGetCapabilitiesResponse) {}

  rpc NodeGetInfo (NodeGetInfoRequest)
    returns (NodeGetInfoResponse) {}
}
```



### Concurrency - 并发访问同一 Volume

In general the Cluster Orchestrator (CO) is responsible for ensuring that there is no more than one call “in-flight” per volume at a given time. However, in some circumstances, the CO MAY lose state (for example when the CO crashes and restarts), and MAY issue multiple calls simultaneously for the same volume. The plugin SHOULD handle this as gracefully as possible. The error code `ABORTED` MAY be returned by the plugin in this case (see the [Error Scheme](https://github.com/container-storage-interface/spec/blob/master/spec.md#error-scheme) section for details).



### Controller Service RPC

#### CreateVolume - 创建 Volume

```c
message CreateVolumeRequest {
  // The suggested name for the storage space. This field is REQUIRED.
  // It serves two purposes:
  // 1) Idempotency(幂等性) - This name is generated by the CO to achieve
  //    idempotency.  The Plugin SHOULD ensure that multiple
  //    `CreateVolume` calls for the same name do not result in more
  //    than one piece of storage provisioned corresponding to that
  //    name. If a Plugin is unable to enforce idempotency, the CO's
  //    error recovery logic could result in multiple (unused) volumes
  //    being provisioned.
    
  //    In the case of error, the CO MUST handle the gRPC error codes
  //    per the recovery behavior defined in the "CreateVolume Errors"
  //    section below.
    
  //    The CO is responsible for cleaning up volumes it provisioned
  //    that it no longer needs. (k8s 需要自己清理不需要的 Volume)
    
  //    If the CO is uncertain whether a volume
  //    was provisioned or not when a `CreateVolume` call fails, the CO
  //    MAY call `CreateVolume` again, with the same name, to ensure the
  //    volume exists and to retrieve the volume's `volume_id` (unless
  //    otherwise prohibited by "CreateVolume Errors").
  // 2) Suggested name - Some storage systems allow callers to specify
  //    an identifier by which to refer to the newly provisioned
  //    storage. If a storage system supports this, it can optionally
  //    use this name as the identifier for the new volume.
  string name = 1;
```



```c
message CreateVolumeResponse {
  // Contains all attributes of the newly created volume that are
  // relevant to the CO along with information required by the Plugin
  // to uniquely identify the volume. This field is REQUIRED.
  Volume volume = 1;
}
```



```c
// Information about a specific volume.
message Volume {
  // The capacity of the volume in bytes. This field is OPTIONAL. If not
  // set (value of 0), it indicates that the capacity of the volume is
  // unknown (e.g., NFS share).
  // The value of this field MUST NOT be negative.
  int64 capacity_bytes = 1;

  // The identifier for this volume, generated by the plugin.
  // This field is REQUIRED.
  // This field MUST contain enough information to uniquely identify
  // this specific volume vs all other volumes supported by this plugin.
  // This field SHALL be used by the CO in subsequent calls to refer to
  // this volume.
  // The SP is NOT responsible for global uniqueness of volume_id across
  // multiple SPs. 不同的 CSI 驱动可能重复 volume_id
  string volume_id = 2;

  // Opaque static properties of the volume. SP MAY use this field to
  // ensure subsequent volume validation and publishing calls have
  // contextual information.
  // The contents of this field SHALL be opaque to a CO.
  // The contents of this field SHALL NOT be mutable.
  // The contents of this field SHALL be safe for the CO to cache.
  // The contents of this field SHOULD NOT contain sensitive
  // information.
  // The contents of this field SHOULD NOT be used for uniquely
  // identifying a volume. The `volume_id` alone SHOULD be sufficient to
  // identify the volume.
  // A volume uniquely identified by `volume_id` SHALL always report the
  // same volume_context.
  // This field is OPTIONAL and when present MUST be passed to volume
  // validation and publishing calls.
  map<string, string> volume_context = 3;

  // If specified, indicates that the volume is not empty and is
  // pre-populated with data from the specified source.
  // This field is OPTIONAL.
  VolumeContentSource content_source = 4;
...
}
```



#### ControllerPublishVolume - 挂载 Volume

A Controller Plugin MUST implement this RPC call if it has `PUBLISH_UNPUBLISH_VOLUME` controller capability. This RPC will be called by the CO **when it wants to place a workload that uses the volume onto a node**. The Plugin SHOULD perform the work that is necessary for making the volume available on the given node. The Plugin MUST NOT assume that this RPC will be executed on the node where the volume will be used.

This operation MUST be idempotent. If the volume corresponding to the `volume_id` has already been published at the node corresponding to the `node_id`, and is compatible with the specified `volume_capability` and `readonly` flag, the Plugin MUST reply `0 OK`.

If the operation failed or the CO does not know if the operation has failed or not, it MAY choose to call `ControllerPublishVolume` again or choose to call `ControllerUnpublishVolume`.

The CO MAY call this RPC for publishing a volume to multiple nodes if the volume has `MULTI_NODE` capability (i.e., `MULTI_NODE_READER_ONLY`, `MULTI_NODE_SINGLE_WRITER` or `MULTI_NODE_MULTI_WRITER`).



```c
message ControllerPublishVolumeRequest {
  // The ID of the volume to be used on a node.
  // This field is REQUIRED.
  string volume_id = 1;

  // The ID of the node. This field is REQUIRED. The CO SHALL set this
  // field to match the node ID returned by `NodeGetInfo`.
  string node_id = 2;

  // Volume capability describing how the CO intends to use this volume.
  // SP MUST ensure the CO can use the published volume as described.
  // Otherwise SP MUST return the appropriate gRPC error code.
  // This is a REQUIRED field.
  VolumeCapability volume_capability = 3;

  // Indicates SP MUST publish the volume in readonly mode.
  // CO MUST set this field to false if SP does not have the
  // PUBLISH_READONLY controller capability.
  // This is a REQUIRED field.
  bool readonly = 4;

  // Secrets required by plugin to complete controller publish volume
  // request. This field is OPTIONAL. Refer to the
  // `Secrets Requirements` section on how to use this field.
  map<string, string> secrets = 5 [(csi_secret) = true];

  // Volume context as returned by SP in
  // CreateVolumeResponse.Volume.volume_context.
  // This field is OPTIONAL and MUST match the volume_context of the
  // volume identified by `volume_id`.
  map<string, string> volume_context = 6;
}

message ControllerPublishVolumeResponse {
  // Opaque static publish properties of the volume. SP MAY use this
  // field to ensure subsequent `NodeStageVolume` or `NodePublishVolume`
  // calls calls have contextual information.
  // The contents of this field SHALL be opaque to a CO.
  // The contents of this field SHALL NOT be mutable.
  // The contents of this field SHALL be safe for the CO to cache.
  // The contents of this field SHOULD NOT contain sensitive
  // information.
  // The contents of this field SHOULD NOT be used for uniquely
  // identifying a volume. The `volume_id` alone SHOULD be sufficient to
  // identify the volume.
  // This field is OPTIONAL and when present MUST be passed to
  // subsequent `NodeStageVolume` or `NodePublishVolume` calls
  map<string, string> publish_context = 1;
}
```





##### ControllerPublishVolume Errors



| Condition                        | gRPC Code             | Description                                                  | Recovery Behavior                                            |
| -------------------------------- | --------------------- | ------------------------------------------------------------ | ------------------------------------------------------------ |
| ...                              | ...                   | ...                                                          | ...                                                          |
| Volume published to another node | 9 FAILED_PRECONDITION | Indicates that a volume corresponding to the specified `volume_id` has already been published at another node and does not have MULTI_NODE volume capability. If this error code is returned, the Plugin SHOULD specify the `node_id` of the node at which the volume is published as part of the gRPC `status.message`. | Node Service RPCCaller SHOULD ensure the specified volume is not published at any other node before retrying with exponential back off. |



### Node Service RPC

#### NodePublishVolume

This RPC is called by the CO when a workload that wants to use the specified volume is placed (scheduled) on a node. The Plugin SHALL assume that this RPC will be executed on the node where the volume will be used.

If the corresponding Controller Plugin has `PUBLISH_UNPUBLISH_VOLUME` controller capability, the CO MUST guarantee that this RPC is called after `ControllerPublishVolume` is called for the given volume on the given node and returns a success.

This operation MUST be idempotent. If the volume corresponding to the `volume_id` has already been published at the specified `target_path`, and is compatible with the specified `volume_capability` and `readonly` flag, the Plugin MUST reply `0 OK`.

If this RPC failed, or the CO does not know if it failed or not, it MAY choose to call `NodePublishVolume` again, or choose to call `NodeUnpublishVolume`.

This RPC MAY be called by the CO multiple times on the same node for the same volume with a possibly different `target_path` and/or other arguments if the volume supports either `MULTI_NODE_...` or `SINGLE_NODE_MULTI_WRITER` access modes (see second table). The possible `MULTI_NODE_...` access modes are `MULTI_NODE_READER_ONLY`, `MULTI_NODE_SINGLE_WRITER` or `MULTI_NODE_MULTI_WRITER`. COs SHOULD NOT call `NodePublishVolume` a second time with a different `volume_capability`. If this happens, the Plugin SHOULD return `FAILED_PRECONDITION`.





