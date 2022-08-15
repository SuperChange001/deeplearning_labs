  ```
  static tflite::MicroMutableOpResolver<11> micro_op_resolver;

  micro_op_resolver.AddMaxPool2D();
  micro_op_resolver.AddConv2D();
  micro_op_resolver.AddReshape();
  micro_op_resolver.AddDepthwiseConv2D();
  micro_op_resolver.AddRelu();
  micro_op_resolver.AddMul();
  micro_op_resolver.AddSub();
  micro_op_resolver.AddPad();
  micro_op_resolver.AddAdd();
  micro_op_resolver.AddMean();
  micro_op_resolver.AddFullyConnected();
```

```
//Model categories go here
const char *kCategoryLabels[kCategoryCount] = {    
    "paper",
    "rock",
    "scissors"
};
```

change the quant.cc