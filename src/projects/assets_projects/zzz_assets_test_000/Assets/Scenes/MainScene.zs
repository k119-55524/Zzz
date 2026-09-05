{
  "name": "MainScene",
  "version": "1.0.0",
  "script": "cc94831d-1c26-4c88-9010-6e9906f880ad",
  "clear": {
    "surfaceMode": "Color",
    "color": [0.0, 0.4706, 0.8431, 1.0],
    "depthMode": "Depth",
    "depth": 1.0,
    "stencilMode": "None",
    "stencil": 0
  },
  "transition": {
    "type": "Instant",
    "duration": 0.0
  },
  "layers": [
    {
      "type": "Layer3D",
      "name": "Main3DLayer",
      "objects": [
        {
          "name": "PlayerObject",
          "domain": "Object",
          "position": [0.0, 0.0, 0.0],
          "script": "6ea9c238-4c23-4b73-be36-8fed508ea612"
        },
        {
          "name": "CubeObject",
          "domain": "Object",
          "position": [0.0, 0.0, 0.0],
          "render": {
            "mesh": "00000000-0000-0000-0000-000000000010"
          }
        }
      ]
    }
  ]
}
