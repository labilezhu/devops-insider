# `rate` vs `increase` functions 

> [https://stackoverflow.com/questions/54494394/do-i-understand-prometheuss-rate-vs-increase-functions-correctly](https://stackoverflow.com/questions/54494394/do-i-understand-prometheuss-rate-vs-increase-functions-correctly)



In an ideal world (where your samples' timestamps are exactly on the second and your rule evaluation happens exactly on the second) `rate(counter[1s])` would return exactly your ICH value and `rate(counter[5s])` would return the average of that ICH and the previous 4. Except the ICH at second 1 is 0, not 1, because no one knows when your counter was zero: maybe it incremented right there, maybe it got incremented yesterday, and stayed at 1 since then. (This is the reason why you won't see an increase the first time a counter appears with a value of 1 -- because your code just created and incremented it.)

`increase(counter[5s])` is exactly `rate(counter[5s]) * 5` (and `increase(counter[2s])` is exactly `rate(counter[2s]) * 2`).



**Now what happens in the real world is that your samples are not collected exactly every second on the second and rule evaluation doesn't happen exactly on the second either.** So if you have a bunch of samples that are (more or less) 1 second apart and you use Prometheus' `rate(counter[1s])`, you'll get no output. That's because what Prometheus does is it takes all the samples in the 1 second range `[now() - 1s, now()]` (which would be a single sample in the vast majority of cases), tries to compute a rate and fails.

If you query `rate(counter[5s])` OTOH, Prometheus will pick all the samples in the range `[now() - 5s, now]` (5 samples, covering approximately 4 seconds on average, say `[t1, v1], [t2, v2], [t3, v3], [t4, v4], [t5, v5]`) and (assuming your counter doesn't reset within the interval) will return `(v5 - v1) / (t5 - t1)`. I.e. it actually computes the rate of increase over ~4s rather than 5s.

`increase(counter[5s])` will return `(v5 - v1) / (t5 - t1) * 5`, so the rate of increase over ~4 seconds, extrapolated to 5 seconds.

Due to the samples not being exactly spaced, both `rate` and `increase` will often return floating point values for integer counters (which makes obvious sense for `rate`, but not so much for `increase`).
