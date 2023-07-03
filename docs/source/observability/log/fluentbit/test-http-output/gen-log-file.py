with open('/tmp/test-fluentbit-1.log', 'w') as f:
  for i in range(10000000):
    f.write('This is line {}.\n'.format(i))